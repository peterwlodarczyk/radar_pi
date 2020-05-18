#include <stdio.h>
#include "pi_common.h"
#include "chart1.h"
#include "ocius/oc_utils.h"
#include "radar_pi.h"
#include "RadarInfo.h"
#include "radar_lib.h"

//#ifndef WX_PRECOMP
//#include "wx/wx.h"
//#endif

#include "pluginmanager.h"
class glChartCanvas : public wxGLCanvas {};

MyFrame* gFrame = nullptr;
PlugInManager* g_pi_manager = nullptr;
extern wxAuiManager* g_pauimgr;


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

///////////////////////////////////////////////////////////////////////////////
// MyFrame
///////////////////////////////////////////////////////////////////////////////
MyFrame::MyFrame(wxWindow* parent) : wxFrame(parent, -1, _("Radar"), wxDefaultPosition, wxSize(500, 500), wxDEFAULT_FRAME_STYLE) {
  // notify wxAUI which frame to use
  m_mgr.SetManagedWindow(this);
  // tell the manager to "commit" all the changes just made
  //m_mgr.Update();
  g_pauimgr = &m_mgr;
  m_glChartCanvas = new wxGLCanvas(this, wxID_ANY, NULL, wxDefaultPosition, wxDefaultSize, 0, wxGLCanvasName, wxNullPalette);
  m_glContext = new wxGLContext(m_glChartCanvas);
  RenderTimer = new wxTimer(this, wxID_HIGHEST);
}

MyFrame::~MyFrame() {
  // deinitialize the frame manager
  g_pauimgr = nullptr;
  m_mgr.UnInit();
}

void MyFrame::OnRenderTimer(wxTimerEvent&) {
  LOG_INFO("[MyFrame::OnRenderTimer].");
  if (g_pi_manager) 
    g_pi_manager->RenderAllGLCanvasOverlayPlugIns(m_glContext, CreatePlugInViewport(), 0);
}

void MyFrame::OnCloseWindow(wxCloseEvent&) {
  LOG_INFO("[MyFrame::OnClose].");
  RenderTimer->Stop();
  if (g_pi_manager) {
    g_pi_manager->UnLoadAllPlugIns();
    delete g_pi_manager;
    g_pi_manager = nullptr;
  }
  gFrame = nullptr;
  Destroy();
}
BEGIN_EVENT_TABLE(MyFrame, wxFrame)
EVT_TIMER(wxID_HIGHEST, MyFrame::OnRenderTimer)
EVT_CLOSE(MyFrame::OnCloseWindow)
END_EVENT_TABLE()

///////////////////////////////////////////////////////////////////////////////
// MyApp
///////////////////////////////////////////////////////////////////////////////
class MyApp : public wxApp {
 public:
  void Close();
  virtual int OnExit();
  virtual bool OnInit();
  FILE* m_LogFile;
  wxLog* m_Logger;
  wxLog* m_OldLogger;
};

DECLARE_APP(MyApp)
#ifdef WIN32
IMPLEMENT_APP(MyApp)
#else
IMPLEMENT_APP_NO_MAIN(MyApp)
#endif

// This is executed upon startup, like 'main()' in non-wxWidgets programs.
bool MyApp::OnInit() {
  // TODO: Get this from configuration
  m_LogFile = fopen(g_OpenCPNLogFilename.c_str(), "a");
  m_Logger = new wxLogStderr(m_LogFile);
  m_OldLogger = wxLog::GetActiveTarget();
  wxLog::SetActiveTarget(m_Logger);
  wxASSERT(m_Logger);

  OC_DEBUG("[MyApp::OnInit]>>");
  LOG_INFO("[MyApp::OnInit]>>");

  g_Platform = new OCPNPlatform(g_ConfigFilename.c_str());

  MyApp::SetAppDisplayName("Radar");

  wxInitAllImageHandlers();

  gFrame = new MyFrame(nullptr);
  gFrame->CreateStatusBar();
  gFrame->SetStatusText(_T("Radar"));
  SetTopWindow(gFrame);

  g_pi_manager = new PlugInManager(gFrame);

  // pConfig = g_Platform->GetConfigObject();
  // pConfig->LoadMyConfig();

  OC_DEBUG("[MyApp::OnInit]<<");
  g_pi_manager->LoadAllPlugIns(true, true);

  static const int INTERVAL = 250;  // milliseconds
  gFrame->RenderTimer->Start(INTERVAL);

  gFrame->Show(true);

  return true;
}

void MyApp::Close() {
  //gFrame->Show(false);
  SetTopWindow(nullptr);
  Yield();
  if (gFrame)
    gFrame->Close(true);

}
int MyApp::OnExit() {
  LOG_INFO("[MyApp::OnExit]>>.");

  LOG_INFO("[MyApp::OnExit]. Attempting to stop logger.");
  wxLog::SetActiveTarget(m_OldLogger);
  delete m_Logger;
  fclose(m_LogFile);
  return 0;
}


// gtk_init_check()
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
    logDir = argv[2];
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
    liveDir = argv[3];
  else
#ifdef WIN32
    liveDir = "c:\\temp\\usv\\live";
#else
    liveDir = "/dev/shm/usv/live";
#endif
  g_OciusLiveDir = liveDir;

#else
bool radar_start(const char* configFilename, const char* logDir, const char* liveDir) {
  int argc = 0;
  char** argv = nullptr;
#endif

  if (strlen(logDir) > 0) {
    g_OciusLogFilename = string(logDir) + '/' + g_OciusLogFilename;
    g_OpenCPNLogFilename = string(logDir) + '/' + g_OpenCPNLogFilename;
  }

  g_ConfigFilename = configFilename;
  OC_DEBUG("[radar_pi::main]");
  OC_DEBUG("g_ConfigFilename=%s.", g_ConfigFilename.c_str());
  OC_DEBUG("g_OciusLogFilename=%s", g_OciusLogFilename.c_str());
  OC_DEBUG("g_OpenCPNLogFilename=%s", g_OpenCPNLogFilename.c_str());
  OC_DEBUG("g_OciusLiveDir=%s", g_OciusLiveDir.c_str());
  OC_DEBUG("DISPLAY=%s", getenv("DISPLAY"));
  try
  {
    int ret = wxEntry(argc, argv);
    OC_DEBUG("wxEntry=%d", ret);
    return ret == 0;
  }
  catch (...)
  {
    printf("Unhandled exception");
    return false;
  }
}

///////////////////////////////////////////////////////////////////////////////
// radar_lib interface
///////////////////////////////////////////////////////////////////////////////

static RadarPlugin::RadarControl* GetRadarControl(int radar) {
  if (radar < RADARS) {
    if (g_pi_manager) {
      RadarPlugin::radar_pi* plugin = g_pi_manager->pPlugin;
      if (plugin) {
        RadarPlugin::RadarInfo* info = plugin->m_radar[radar];
        if (info) {
          RadarPlugin::RadarControl* control = info->m_control;
          return control;
        }
      }
    }
  }
  return nullptr;
}

void radar_stop() { 
  if (gFrame)
    wxGetApp().Close();
}

bool radar_set_tx(int radar, bool on) { 
  auto controller = GetRadarControl(radar);
  if (controller) {
    if (on)
      controller->RadarTxOn();
    else 
      controller->RadarTxOff();
  }

  return true;
}

bool radar_get_tx(int radar) {
  return radar;
}

int radar_set_range(int radar, int range){
  auto controller = GetRadarControl(radar);
  if (controller) {
    controller->SetRange(range);
  }
  return range;
}

int radar_get_range(int radar) {
  return 0;
}

int radar_set_control(int radar, const char* control, int value) {
  //auto controller = GetRadarControl(radar);
  //controller->SetControl(range);
  return value;
}

int radar_get_control(int radar, const char* cont6rol) {
  return 0;
}

RadarState radar_get_state(int radar) {
  RadarState state = { false };
  return state;
}
