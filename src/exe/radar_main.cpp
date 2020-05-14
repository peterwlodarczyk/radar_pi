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

#ifdef WIN32
#define LOG_FILENAME "radar.log"
#else
#define LOG_FILENAME "/tmp/usv/comp/radar.log"
#endif


bool RadarApp::OnInit() {
  // TODO: Get this from configuration
  m_LogFile = fopen(LOG_FILENAME, "a");
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

  m_PluginManager = new PlugInManager(frame);
  OC_DEBUG("[RadarApp::OnInit]<<");
  m_PluginManager->LoadAllPlugIns(true, true);

#ifdef RENDER_OVERLAY
  static const int INTERVAL = 1000;  // milliseconds
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
#else
int radar_start() {
  int argc = 0;
  char** argv = nullptr;
#endif
  printf("[radar_pi::main]\n");
  return wxEntry(argc, argv);
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
