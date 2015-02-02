#pragma once

#ifndef __AFXWIN_H__
	#error 'stdafx.h' muss vor dieser Datei in PCH eingeschlossen werden.
#endif

#include "resource.h"		// Hauptsymbole

//--- Standard includes ---------------------------------------------------------------------
#include <memory>

//--- Utility classes -----------------------------------------------------------------------
#include "utils/auIniFile.h"

//--- Application related -------------------------------------------------------------------
#include "WndOpenGL.h"
#include "SimThread.h"


//-------------------------------------------------------------------------------------------
class CSimApp : public CWinApp
{
public:
	CSimApp();
  virtual ~CSimApp();
	virtual BOOL InitInstance();

  DECLARE_MESSAGE_MAP()

private:
  std::auto_ptr<CWndOpenGL> m_pWnd;
  std::auto_ptr<SimThread> m_pThread;
  au::IniFile m_Config;
};

extern CSimApp theApp;