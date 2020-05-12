#include <stdio.h>
#include "radar_pi.h"
#include "ocius/oc_utils.h"
#include "pluginmanager.h"
#include "chart1.h"

// For compilers that don't support precompilation, include "wx/wx.h"
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#if 1

class RadarApp : public wxApp {
 public:
  virtual int OnExit();
  virtual bool OnInit();
  FILE* m_LogFile;
  wxLog* m_Logger;
  wxLog* m_OldLogger;
  PlugInManager* m_PluginManager;
};

DECLARE_APP(RadarApp)
IMPLEMENT_APP(RadarApp)

// This is executed upon startup, like 'main()' in non-wxWidgets programs.
int RadarApp::OnExit() { 
  LOG_INFO("[RadarApp::OnExit]>>.");

  m_PluginManager->UnLoadAllPlugIns();
  delete m_PluginManager;

  LOG_INFO("[RadarApp::OnExit]. Attempting to stop logger.");
  wxLog::SetActiveTarget(m_OldLogger);
  delete m_Logger;
  fclose(m_LogFile);
  return 0;
}

bool RadarApp::OnInit() {
  m_LogFile = fopen("radar.log", "a");
  m_Logger = new wxLogStderr(m_LogFile);
  m_OldLogger = wxLog::GetActiveTarget();
  wxLog::SetActiveTarget(m_Logger);
  wxASSERT(m_Logger);

  OC_DEBUG("[RadarApp::OnInit]>>");
  LOG_INFO("[RadarApp::OnInit]>>");

  MyFrame* frame = new MyFrame(nullptr);
  frame->CreateStatusBar();
  frame->SetStatusText(_T("Radar"));
  frame->Show(true);
  SetTopWindow(frame);

  m_PluginManager = new PlugInManager(frame);
  OC_DEBUG("[RadarApp::OnInit]<<");
  m_PluginManager->LoadAllPlugIns(true, true);
  printf("radar_exe\n");

  return true;
}
#else
class DerivedApp : public wxApp
{
public:
    virtual bool OnInit();
};
wxIMPLEMENT_APP(DerivedApp);
bool DerivedApp::OnInit()
{
    wxFrame *the_frame = new wxFrame(NULL, -1, argv[0]);
    the_frame->Show(true);
    return true;
}
#endif
