#ifndef SIM_THREAD_H
#define SIM_THREAD_H

//-------------------------------------------------------------------------------------------
#include <afxmt.h>          
#include <memory>
#include <string>
#include <vector>

//-------------------------------------------------------------------------------------------
#include "utils/auIniFile.h"
#include "utils/muGeneric.h"
#include "utils/muVector.h"
#include "utils/muBlockMatrix.h"

//---------------------------------------------------------------------------------------
#include "SimPend.h"

//---------------------------------------------------------------------------------------
// Forward declarations
class CWndOpenGL;

//-------------------------------------------------------------------------------------------
/** \brief Simulation Launcher. */
class SimThread
{
public:
    SimThread(CWndOpenGL *pWnd, const au::IniFile &iniFile);
    virtual ~SimThread();
    virtual void Start();
    virtual void HandleMouseClick(int x, int y); 
    virtual void HandleMouseMove(int x, int y); 
    virtual void Finalize();
    virtual void SetPath(const std::string &sName);
    virtual void SetName(const std::string &sName);
    virtual const std::string& GetName() const;
    virtual const std::string& GetPath() const;
    const SimImpl* GetSim() const;
    static UINT ThreadMain(LPVOID lpParam);

private:
    const std::auto_ptr<SimImpl> m_pSim;
    std::string m_sPath;
    std::string m_sName;
    std::vector<HANDLE> m_vThreadTable;
//    std::vector<CWinThread*> m_vThreads;
    HANDLE m_hCloseEvent;

    CWndOpenGL *m_pWnd;
    CCriticalSection  m_KillLock;
    volatile bool m_bRunning;
    bool m_bShowTraces;

    SimThread(const SimThread &ref);
    SimThread& operator=(const SimThread &ref);
};

#endif // include guard