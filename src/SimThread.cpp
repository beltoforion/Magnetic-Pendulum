#include "stdafx.h"

//--- Standard includes ---------------------------------------------------------------------
#include <cmath>
#include <string>
#include <sstream>

//--- Utility classes -----------------------------------------------------------------------
#include "utils/auThreads.h"

//--- Simulation implementation -------------------------------------------------------------
#include "SimThread.h"
#include "SimPend.h"
#include "WndOpenGL.h"


//-------------------------------------------------------------------------------------------
SimThread::SimThread(CWndOpenGL *pWnd, const au::IniFile &iniFile)
  :m_pSim( new SimImpl(pWnd, iniFile) )
  ,m_pWnd( pWnd )
  ,m_bRunning(true)
  ,m_sPath()
  ,m_sName()
  ,m_vThreadTable()
  ,m_hCloseEvent(NULL)
  ,m_bShowTraces(false)
{
  ASSERT(pWnd);
  pWnd->SetSim(this);
}

//-------------------------------------------------------------------------------------------
/** \brief Kill all Threads if not already done. */
SimThread::~SimThread()
{
  // Don't call Finalize here, The OpenGL window must call Finalize
  // otherwise the threads accesses a detroyed window.
}

//-------------------------------------------------------------------------------------------
/** \brief Terminate all running threads. 

  Blocks execution until all threads are terminated.
*/
void SimThread::Finalize()
{
  au::AutoLock<CCriticalSection> lock(&m_KillLock);

  if (m_bRunning && m_vThreadTable.size()>=0)
  {
    m_bRunning = false;
    SetEvent ( m_hCloseEvent );
    Sleep(0);

    m_pSim->DumpToFile( GetPath(), GetName() );
    DWORD nThreads( (DWORD)m_vThreadTable.size() );
    WaitForMultipleObjects( nThreads,
                            &m_vThreadTable[0], 
                            TRUE, 
                            INFINITE);
    m_vThreadTable.clear();
  } // if not running
}

//-------------------------------------------------------------------------------------------
void SimThread::SetPath(const std::string &sPath)
{
  m_sPath = sPath;
}

//-------------------------------------------------------------------------------------------
void SimThread::SetName(const std::string &sName)
{
  m_sName = sName;
}

//-------------------------------------------------------------------------------------------
const std::string& SimThread::GetName() const
{
  return m_sName;
}

//-------------------------------------------------------------------------------------------
const std::string& SimThread::GetPath() const
{
  return m_sPath;
}

//-------------------------------------------------------------------------------------------
const SimImpl* SimThread::GetSim() const
{
  return m_pSim.get();
}

//-------------------------------------------------------------------------------------------
void SimThread::HandleMouseClick(int /*x*/, int /*y*/)
{
  m_bShowTraces ^= true;
}

//-------------------------------------------------------------------------------------------
void SimThread::HandleMouseMove(int x, int y)
{
  if (m_bShowTraces)
    return;

  CWndOpenGL::PaintLock lock(m_pWnd);

  // convert window client coordinates to physical position
  double width(0), height(0);
  m_pSim->WinCoordToModel(x, y, width, height);
  m_pSim->DrawModel();
  SimImpl::trace_buf_type vTrace;
  int idx( m_pSim->Calc( mu::vec2d_type(width, height),
                         mu::vec2d_type(0,0), 
                         &vTrace) );

  if ( idx >= 0 )
  {
    const ISource *pSrc( m_pSim->GetMagnet(idx) );
    glColor3ub( (GLubyte)pSrc->GetRed(), 
                (GLubyte)pSrc->GetGreen(), 
                (GLubyte)pSrc->GetBlue() );
  }
  else
    glColor3ub( (GLubyte)255, (GLubyte)255, (GLubyte)255 );

  m_pWnd->DrawLineStrip(vTrace, 1);
}

//-------------------------------------------------------------------------------------------
void SimThread::Start()
{
  m_pSim->Restore(GetPath(), GetName());
  m_pSim->DrawModel();

  CWinThread *pNewThread(NULL);
  
  // get number of processors
  SYSTEM_INFO sysinfo;
  GetSystemInfo( &sysinfo );
  DWORD &nProc(sysinfo.dwNumberOfProcessors);
  int nThreads( (m_pSim->GetThreadCount()==-1) ? nProc : m_pSim->GetThreadCount() );
  
  // Create the worker threads
  m_vThreadTable.resize(nThreads);
  m_hCloseEvent = CreateEvent (NULL, TRUE, FALSE, NULL); // "SIM_PEND_CLOSE");
  for (int i=0; i<nThreads; ++i)
  {
    pNewThread = AfxBeginThread ( ThreadMain,
                                  reinterpret_cast<LPVOID>(this),
                                  THREAD_PRIORITY_NORMAL,
                                  0,  // default stack size
                                  0,  // start immediately
                                  NULL );
    if (pNewThread)
    {
      m_vThreadTable[i] = pNewThread->m_hThread;
      pNewThread->m_bAutoDelete = FALSE;
    }

    DWORD dwErr( SetThreadIdealProcessor( pNewThread->m_hThread, i % nProc) );
/*
    if (dwErr==-1)
    {
      std::stringstream ss;
      ss << "Cant set preferred processor for thread " << i;
      throw std::runtime_error( ss.str().c_str() );
    } 
*/
  } // for all threads to start
}

//-------------------------------------------------------------------------------------------
UINT SimThread::ThreadMain(LPVOID lpParam)
{
  TRACE("Thread started.\n");

  static CCriticalSection DataLock;
  SimThread *pSelf( static_cast<SimThread*>(lpParam) );
  ASSERT( pSelf->m_pSim.get() );

  try
  {
    const int nTraceStep = 5;

    SimImpl &sim( *(pSelf->m_pSim) );
    SimImpl::trace_buf_type vTrace;

    int nCols(0), nRows(0);
    int y(0), 
        hLine(0);

    sim.QuerySimGrid(nCols, nRows);
    vTrace.reserve(20000);

    while( pSelf->m_bRunning )
    {
      // query line to process
      {
        au::AutoLock<CCriticalSection> lock(&DataLock);
        hLine = sim.QueryNextLine(y);
        if (y>(nCols-1) || y<0)
          break;
      }

      // Calculate a single trace
      mu::vec2d_type start_pos(0, 0),
                     start_vel(0, 0);
      mu::ivec2d_type grid_pos(0, 0);

      SimImpl::trace_buf_type *pvTrace(NULL);
      for (int x=0; x<nCols && pSelf->m_bRunning; ++x)
      {
        sim.GridCoordToModel(x, y, start_pos[0], start_pos[1]);     

        pvTrace = (pSelf->m_bShowTraces && (x % nTraceStep == 0) ) ? &vTrace : NULL;
        int idx( sim.Calc( start_pos, start_vel, &vTrace ) );
        if (pvTrace)
          sim.DrawTrace(vTrace, idx);
      } // for all points in the line

      if (pSelf->m_bRunning)
      {
        au::AutoLock<CCriticalSection> lock(&DataLock);
        sim.ScreenRefresh(y);  // Write line data to frame buffer
        sim.DrawModel();       // Display data
        sim.FlagAsDone(hLine);

        if (sim.IsDone() && sim.GetBatchMode())
        {
          // SendMessage would cause a deadlock
          pSelf->m_pWnd->PostMessage(WM_CLOSE);
          break;
        }
      } // if thread is running
    } // while running
  }
  catch(std::exception &e)
  {
    AfxMessageBox( e.what() );
  }
  catch(...)
  {
    AfxMessageBox("unexpected exception: closing thread.");
  }

  // Wait for termination event
  WaitForSingleObject(pSelf->m_hCloseEvent, INFINITE);
  return 0;
}
