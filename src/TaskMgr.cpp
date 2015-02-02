#include "stdafx.h"
#include "TaskMgr.h"

#include <stdexcept>
#include <algorithm>
#include <functional> 
#include <fstream>


//-------------------------------------------------------------------------------------------
TaskMgr::TaskMgr()
  :m_nNextIdx(0)
  ,m_vLinesToCalc()
{}

//-------------------------------------------------------------------------------------------
TaskMgr::~TaskMgr()
{}

//-------------------------------------------------------------------------------------------
/** \brief Reset vector holding indices of lines waiting for calculation. */
void TaskMgr::Reset(int nLines)
{
  m_vLinesToCalc.assign(nLines, -1);
  for (int i=0; i<nLines; ++i)
    m_vLinesToCalc[i] = i;

  m_nNextIdx = 0;
}

//-------------------------------------------------------------------------------------------
/** \brief Return index of next line to calculate or -1 if no lines are left. 
*/
int TaskMgr::GetNextLine(int &nLineIdx)
{
  if (m_nNextIdx<0)
    return -1;

  nLineIdx = m_nNextIdx++;
  return (nLineIdx < (int)m_vLinesToCalc.size()) ? m_vLinesToCalc[nLineIdx] : -1;
}

//-------------------------------------------------------------------------------------------
int TaskMgr::GetNumLinesDone() const
{
  return m_nNextIdx;
}

//-------------------------------------------------------------------------------------------
/** \brief Flag a line index as caqlculated. */
void TaskMgr::FlagAsCalculated(int nLine)
{
  if (nLine>=0 && nLine<(int)m_vLinesToCalc.size())
    m_vLinesToCalc[nLine] = -1;
}

//-------------------------------------------------------------------------------------------
bool TaskMgr::IsDone() const
{
  using std::vector;
  using std::max_element;
 
  vector<int>::const_iterator it( max_element( m_vLinesToCalc.begin(), 
                                               m_vLinesToCalc.end() ) );
  int nMax(*it);
  return nMax == -1;
}

//-------------------------------------------------------------------------------------------
void TaskMgr::SaveState(const std::string &sFile) const
{
  std::ofstream ofs_pos( sFile.c_str(), std::ios::out | std::ios::binary);
  ofs_pos.write((const char*)(&m_vLinesToCalc[0]), (std::streamsize)(m_vLinesToCalc.size() * sizeof(int)) );
  ofs_pos.close();
}

//-------------------------------------------------------------------------------------------
void TaskMgr::RestoreState(const std::string &sFile)
{
  std::ifstream ofs_pos( sFile.c_str(), std::ios::in | std::ios::binary);
  if (!ofs_pos)
    throw std::runtime_error("can't restore line state.");

  m_vLinesToCalc.assign( m_vLinesToCalc.size(), -1 );
  ofs_pos.seekg(0, std::ios::beg);
  ofs_pos.read((char*)(&m_vLinesToCalc[0]), (std::streamsize)(m_vLinesToCalc.size() * sizeof(int)) );
  ofs_pos.close();

  // Find out which line is the next one
  std::sort(m_vLinesToCalc.begin(), m_vLinesToCalc.end(), std::less<int>());
  for (int i=0; i<(int)(m_vLinesToCalc.size()-1) && m_vLinesToCalc[i]==-1; ++i)
    m_nNextIdx = m_vLinesToCalc[i+1];

  if (m_nNextIdx<0)
    m_nNextIdx = (int)m_vLinesToCalc.size()-1;
}
