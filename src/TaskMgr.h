#ifndef TASK_MGR_H
#define TASK_MGR_H

#include <vector>
#include <string>

//-------------------------------------------------------------------------------------------
/** \brief A class keeping trak of lines to calculate. */
class TaskMgr
{
public:
    TaskMgr();
    ~TaskMgr();

    void Reset(int nLines);
    int GetNextLine(int &nLineIdx);
    int GetNumLinesDone() const;
    void FlagAsCalculated(int nLine);
    bool IsDone() const;
    void SaveState(const std::wstring &sFile) const;
    void RestoreState(const std::wstring &sFile);

private:
    int m_nNextIdx;     ///< Total number of lines
    std::vector<int> m_vLinesToCalc;
};

#endif