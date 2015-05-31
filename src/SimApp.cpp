#include "stdafx.h"
#include <sstream>
#include <memory>

//--- Utility classes -----------------------------------------------------------------------
#include "utils/wuParameter.h"
#include "utils/suUtility.h"
#include "utils/auThreads.h"
#include "utils/utWideExceptions.h"

//--- My includes ---------------------------------------------------------------------------
#include "SimApp.h"
#include "SimPend.h"
#include "SimThread.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// Das einzige CChaosApp-Objekt. Kaufen Sie keine billigen Fälschungen!
CSimApp theApp;


//-------------------------------------------------------------------------------------------
// CChaosApp
BEGIN_MESSAGE_MAP(CSimApp, CWinApp)
    ON_COMMAND(ID_HELP, CSimApp::OnHelp)
END_MESSAGE_MAP()


//-------------------------------------------------------------------------------------------
// CChaosApp-Erstellung
CSimApp::CSimApp()
:CWinApp()
, m_pWnd()
, m_pThread()
, m_Config()
{}

//-------------------------------------------------------------------------------------------
CSimApp::~CSimApp()
{
    m_pThread.reset(0);
}

//-------------------------------------------------------------------------------------------
// CChaosApp Initialisierung
BOOL CSimApp::InitInstance()
{
    CWinApp::InitInstance();

    try
    {
        std::wstring sFile(CWinApp::m_lpCmdLine);
        if (sFile.length() == 0)
        {
            throw utils::wruntime_error(_T("usage:  SimPend config.cfg\n\nplease provide a config file."));
        }

        sFile = su::unquote(sFile);
        m_Config.Load(sFile, au::IniFile::eIGNORE_CASE);

        // Create the OpenGL window
        m_pWnd.reset(new CWndOpenGL);
        m_pMainWnd = m_pWnd.get();

        // initialize thread and simulation
        int w(0), h(0), cols(0), rows(0);
        double  fw(0), fh(0);
        m_pThread.reset(new SimThread(m_pWnd.get(), m_Config));
        m_pThread->GetSim()->QueryWinDim(w, h);
        m_pThread->GetSim()->QuerySimGrid(cols, rows);
        m_pThread->GetSim()->QuerySimDim(fw, fh);
        CRect rect(0, 0, w, h);
        m_pWnd->Create(rect, cols, rows, fw, fh);

        // I need to resize the window because the border size is not accounted for in rect
        m_pWnd->CalcWindowRect(rect);
        m_pWnd->MoveWindow(0, 0, rect.Width(), rect.Height());
        // Now once we have the real Size we can initialize OpenGL
        m_pWnd->InitGL();
        m_pWnd->InitFrameBuf();

        wchar_t szName[2048], szDrive[2048], szDir[2048];
        _wsplitpath(sFile.c_str(), szDrive, szDir, szName, NULL);
        std::wstringstream ss;
        ss << szDrive << szDir; // << szName;

        m_pThread->SetPath(ss.str());
        m_pThread->SetName(szName);
        m_pThread->Start();

        return TRUE;
    }
    catch (utils::wruntime_error &e)
    {
        AfxMessageBox(e.message().c_str());
        return FALSE;
    }
    catch (std::exception &)
    {
        AfxMessageBox(_T("unexpected exception"));
        return FALSE;
    }
}
