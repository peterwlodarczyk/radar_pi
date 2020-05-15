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
#define RENDER_OVERLAY

class glChartCanvas : public wxGLCanvas {
};
  
class RadarApp : public wxApp {
 public:
  virtual int OnExit();
  virtual bool OnInit();
  FILE* m_LogFile;
  wxLog* m_Logger;
  wxLog* m_OldLogger;
  PlugInManager* m_PluginManager;
  wxTimer* m_timer;

#ifdef RENDER_OVERLAY
  wxGLCanvas* m_glChartCanvas;
  wxGLContext* m_glContext;
  void OnRenderTimer(wxTimerEvent& event);
#endif

  DECLARE_EVENT_TABLE();
};

DECLARE_APP(RadarApp)
#ifdef WIN32
IMPLEMENT_APP(RadarApp)
#else
IMPLEMENT_APP_NO_MAIN(RadarApp)
#endif

BEGIN_EVENT_TABLE(RadarApp, wxApp)
#ifdef RENDER_OVERLAY
EVT_TIMER(wxID_HIGHEST, RadarApp::OnRenderTimer)
#endif
END_EVENT_TABLE() 

#ifdef RENDER_OVERLAY
 void RadarApp::OnRenderTimer(wxTimerEvent&) {
  LOG_INFO("[RadarApp::OnRenderTimer].");
  m_PluginManager->RenderAllGLCanvasOverlayPlugIns(m_glContext, CreatePlugInViewport(), 0);
}
#endif

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
  // TODO: Get this from configuration
  m_LogFile = fopen(g_OpenCPNLogFilename.c_str(), "a");
  m_Logger = new wxLogStderr(m_LogFile);
  m_OldLogger = wxLog::GetActiveTarget();
  wxLog::SetActiveTarget(m_Logger);
  wxASSERT(m_Logger);

  OC_DEBUG("[RadarApp::OnInit]>>");
  LOG_INFO("[RadarApp::OnInit]>>");

  wxInitAllImageHandlers();

  MyFrame* frame = new MyFrame(nullptr);
  frame->CreateStatusBar();
  frame->SetStatusText(_T("Radar"));
  frame->Show(true);
  SetTopWindow(frame);

#ifdef RENDER_OVERLAY
  m_glChartCanvas = new wxGLCanvas(frame, wxID_ANY, NULL, wxDefaultPosition, wxDefaultSize, 0, wxGLCanvasName, wxNullPalette);
  m_glContext = new wxGLContext(m_glChartCanvas);
#endif

  g_Platform = new OCPNPlatform(g_ConfigFilename.c_str());
  m_PluginManager = new PlugInManager(frame);
  OC_DEBUG("[RadarApp::OnInit]<<");
  m_PluginManager->LoadAllPlugIns(true, true);

#ifdef RENDER_OVERLAY
  static const int INTERVAL = 250;  // milliseconds
  m_timer = new wxTimer(this, wxID_HIGHEST);
  m_timer->Start(INTERVAL);
#endif

  return true;
}

//gtk_init_check()
// sudo apt-get install -y xvfb
// Xvfb :99 & DISPLAY=:99 ./radar_pi

#ifdef RADAR_EXE
int main(int argc, char* argv[]) {
  const char* configFilename;
  if (argc > 1) 
    configFilename = argv[1];
  else
#ifdef WIN32
    configFilename = "C:\\ProgramData\\opencpn\\opencpn.ini";
#else
    configFilename = "~/.opencpn/opencpn.conf";
#endif

  const char* logDir;
  if (argc > 2)
    logDir  = argv[2];
  else
#ifdef WIN32
    logDir = "C:\\ProgramData\\opencpn";
#else
    logDir = "/tmp";
#endif
  g_OciusLogFilename = string(logDir) + "/radar-ocius.log";
  g_OpenCPNLogFilename = string(logDir) + "/radar-opencpn.log";

  const char* liveDir;
  if (argc > 3)
    liveDir  = argv[3];
  else
#ifdef WIN32
    liveDir = "c:\\temp\\usv\\live";
#else
    liveDir = "/dev/shm/usv/live";
#endif
  g_OciusLiveDir = liveDir;

#else
int radar_start(const char* configFilename, const char* logDir, const char* liveDir) {
  int argc = 0;
  char** argv = nullptr;
#endif

  g_ConfigFilename = configFilename;
  OC_DEBUG("[radar_pi::main]");
  OC_DEBUG("g_ConfigFilename=%s.", g_ConfigFilename.c_str());
  OC_DEBUG("g_OciusLogFilename=%s", g_OciusLogFilename.c_str());
  OC_DEBUG("g_OpenCPNLogFilename=%s", g_OpenCPNLogFilename.c_str());
  OC_DEBUG("g_OciusLiveDir=%s", g_OciusLiveDir.c_str());
  OC_DEBUG("DISPLAY=%s", getenv("DISPLAY"));
  int ret = wxEntry(argc, argv);
  OC_DEBUG("wxEntry=%d", ret);
  return ret;
}

void radar_stop() { 
  wxGetApp().GetTopWindow()->Close(); 
}

/*
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
*/
