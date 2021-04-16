#include <stdio.h>
#include "pi_common.h"
#include "chart1.h"
#include "ocius/oc_utils.h"
#include "radar_pi.h"
#include "RadarInfo.h"
#include "radar_lib.h"
#include "GuardZone.h"
#include "RadarControlItem.h" //control items
#include "ControlsDialog.h" //control buttons
#include "RadarMarpa.h" //contains arpatarget class
#include "Kalman.h" //the Polar def.

static ExtendedPosition RadarPolar2Pos(uint8_t radar, const RadarPlugin::Polar& pol, const ExtendedPosition& own_ship);
static RadarPlugin::Polar RadarPos2Polar(uint8_t radar, const ExtendedPosition& p, const ExtendedPosition& own_ship);

//#ifndef WX_PRECOMP
//#include "wx/wx.h"
//#endif

#include "pluginmanager.h"
class glChartCanvas : public wxGLCanvas {};

MyFrame* gFrame = nullptr;
PlugInManager* g_pi_manager = nullptr;
extern wxAuiManager* g_pauimgr;

using namespace RadarPlugin;

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
MyFrame::MyFrame(wxWindow* parent) : wxFrame(parent, -1, _("Radar"), wxDefaultPosition, wxSize(20, 20), wxDEFAULT_FRAME_STYLE) {
  // notify wxAUI which frame to use
  m_mgr.SetManagedWindow(this);
  // tell the manager to "commit" all the changes just made
  //m_mgr.Update();
  g_pauimgr = &m_mgr;
  m_glChartCanvas = new wxGLCanvas(this, wxID_ANY, NULL, wxDefaultPosition, wxSize(100,100), 0, wxGLCanvasName, wxNullPalette);
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
  if ( getenv("DISPLAY") == nullptr )
    setenv("DISPLAY", ":0", 1);
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
  const char* liveDir;
  if (argc > 3)
    liveDir = argv[3];
  else
#ifdef WIN32
    liveDir = "c:\\temp\\usv\\live";
#else
    liveDir = "/dev/shm/usv/live";
#endif

#else
bool radar_start(const char* configFilename, const char* logDir, const char* liveDir) {
  int argc = 0;
  char** argv = nullptr;
#endif

  if (strlen(logDir) > 0) {
    g_OciusLogFilename = string(logDir) + '/' + g_OciusLogFilename;
    g_OpenCPNLogFilename = string(logDir) + '/' + g_OpenCPNLogFilename;
  }

  g_OciusLiveDir = liveDir;
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

RadarPlugin::ControlType ControlTypeStringToEnum(const char* name) {
  if (strcmp(name, "antenna_forward") == 0)
    return CT_ANTENNA_FORWARD;
  if (strcmp(name, "antenna_starboard") == 0)
    return CT_ANTENNA_STARBOARD;
  if (strcmp(name, "main_bang_size") == 0)
    return CT_MAIN_BANG_SIZE;
  if (strcmp(name, "orientation") == 0)
    return CT_ORIENTATION;
  if (strcmp(name, "center_view") == 0)
	  return CT_CENTER_VIEW;
  if (strcmp(name, "overlay_canvas") == 0)
	  return CT_OVERLAY_CANVAS;
  if (strcmp(name, "target_on_ppi") == 0)
	  return CT_TARGET_ON_PPI;
  if (strcmp(name, "refreshrate") == 0)
	  return CT_REFRESHRATE;
  if (strcmp(name, "target_trails") == 0)
	  return CT_TARGET_TRAILS;
  if (strcmp(name, "timed_idle") == 0)
	  return CT_TIMED_IDLE;
  if (strcmp(name, "timed_run") == 0)
	  return CT_TIMED_RUN;
  if (strcmp(name, "trails_motion") == 0)
	  return CT_TRAILS_MOTION;
  if (strcmp(name, "antenna_height") == 0)
	  return CT_ANTENNA_HEIGHT;
  if (strcmp(name, "bearing_alignment") == 0)
	  return CT_BEARING_ALIGNMENT;
  if (strcmp(name, "gain") == 0)
	  return CT_GAIN;
  if (strcmp(name, "interference_rejection") == 0)
	  return CT_INTERFERENCE_REJECTION;
  if (strcmp(name, "local_interference_rejection") == 0)
	  return CT_LOCAL_INTERFERENCE_REJECTION;
  if (strcmp(name, "noise_rejection") == 0)
	  return CT_NOISE_REJECTION;
  if (strcmp(name, "no_transmit_end") == 0)
	  return CT_NO_TRANSMIT_END;
  if (strcmp(name, "no_transmit_start") == 0)
	  return CT_NO_TRANSMIT_START;
  if (strcmp(name, "rain") == 0)
	  return CT_RAIN;
  if (strcmp(name, "range") == 0)
	  return CT_RANGE;
  if (strcmp(name, "scan_speed") == 0)
	  return CT_SCAN_SPEED;
  if (strcmp(name, "sea") == 0)
	  return CT_SEA;
  if (strcmp(name, "ftc") == 0)
	  return CT_FTC;
  if (strcmp(name, "side_lobe_suppression") == 0)
	  return CT_SIDE_LOBE_SUPPRESSION;
  if (strcmp(name, "target_boost") == 0)
	  return CT_TARGET_BOOST;
  if (strcmp(name, "target_expansion") == 0)
	  return CT_TARGET_EXPANSION;
  if (strcmp(name, "target_separation") == 0)
	  return CT_TARGET_SEPARATION;
  if (strcmp(name, "transparency") == 0)
	  return CT_TRANSPARENCY;
  if (strcmp(name, "doppler") == 0)
	  return CT_DOPPLER;
  return CT_NONE;
}


///////////////////////////////////////////////////////////////////////////////
// radar_lib interface
///////////////////////////////////////////////////////////////////////////////

static RadarPlugin::radar_pi* GetRadarPlugin() {
  if (g_pi_manager) {
    RadarPlugin::radar_pi* plugin = g_pi_manager->pPlugin;
    return plugin;
  }
  return nullptr;
}

static RadarPlugin::RadarInfo* GetRadarInfo(int radar) {
  if (radar < RADARS) {
    if (g_pi_manager) {
      RadarPlugin::radar_pi* plugin = g_pi_manager->pPlugin;
      if (plugin) {
        RadarPlugin::RadarInfo* info = plugin->m_radar[radar];
        return info;
      }
    }
  }
  return nullptr;
}

static RadarPlugin::RadarControl* GetRadarController(int radar) {
  if (radar < RADARS) {
    if (g_pi_manager) {
      RadarPlugin::radar_pi* plugin = g_pi_manager->pPlugin;
      if (plugin) {
        RadarPlugin::RadarInfo* info = plugin->m_radar[radar];
        if (info) {
          RadarPlugin::RadarControl* control = info->m_control;
          if (control && control->IsInitialised())
            return control;
          else {
            return nullptr;
          }
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

::RadarStatus radar_get_status() {
  // TODO: Map to more states. Detect when radar isn't talking.
  if (radar_get_state(0) == OC_RADAR_OFF)
    return OC_RADAR_STATUS_INACTIVE;
  else
    return OC_RADAR_STATUS_HEALTHY;
}
bool radar_set_enable(uint8_t radar, bool enable) {
  return true;
}
bool radar_get_enable(uint8_t radar) {
  return true;
}

::RadarState radar_get_state(uint8_t radar) {
  auto info = GetRadarInfo(radar);
  if (info) {
    return static_cast<::RadarState>(info->m_state.GetValue());
  }
  return OC_RADAR_OFF;
}

bool radar_set_tx(uint8_t radar, bool on) { 
  auto controller = GetRadarController(radar);
  if (controller) {
    if (on)
      controller->RadarTxOn();
    else 
      controller->RadarTxOff();
    return true;
  }
  else {
    return false;
  }

}

bool radar_get_tx(uint8_t radar) {
  auto info = GetRadarInfo(radar);
  if (info)
    return info->m_state.GetValue() == OC_RADAR_TRANSMIT;
  else
    return false;
}

bool radar_config_save() { 
  OC_DEBUG("[%s]", __func__);
  auto plugin = GetRadarPlugin();
  if (plugin)
    return plugin->SaveConfig();
  else
    return false;
}

double radar_set_range(uint8_t radar, double range) {
  auto controller = GetRadarController(radar);
  if (controller) {
    controller->SetRange(range);
    range = controller->GetRange();
  }
  else {
    range = 0;
  }
  OC_DEBUG("[%s]=%f.", __func__, range);
  return range;
}

double radar_get_range(uint8_t radar) {
  int range = 0;
  auto controller = GetRadarController(radar);
  if (controller != nullptr) {
    range = controller->GetRange();
  }
  OC_DEBUG("[%s]=%d.", __func__, range);
  return range;
}

bool radar_set_item_control(uint8_t radar, const char* control_string, ::RadarControlState state, int32_t value){
  //note it might be possible to replace the set_control calls here with virtually pressing the associated button.
  bool r = false;
  auto info = GetRadarInfo(radar);
  RadarControlItem item; //todo, fake an item with the value we need.
  item.Update(value, (RadarPlugin::RadarControlState) state);
  //RadarControlButton button; //i don't think this gets used by the target trails function - so make it an empty button.

  if (info != nullptr)
  {
    ControlType control_enum = ControlTypeStringToEnum(control_string);
    r = info->SetControlValue(control_enum, item, nullptr); //fake a button and hope it doesn't break things?
  }
  OC_DEBUG("[%s]=%d.control_item=%s.state=%d,value=%d.", __func__, r, control_string, state, value);
  return r;
}

bool radar_set_control(uint8_t radar, const char* control_string, ::RadarControlState state, int32_t value) {
  bool r = false;
  auto controller = GetRadarController(radar);
  if (controller != nullptr)
  {
    ControlType control_enum = ControlTypeStringToEnum(control_string);
    r = controller->SetControlValue(control_enum, (RadarPlugin::RadarControlState)state, value);
  }
  OC_DEBUG("[%s]=%d.control=%s.state=%d,value=%d.", __func__, r, control_string, state, value);
  return r;
}

bool radar_get_control(uint8_t radar, const char* control_string, ::RadarControlState* state, int32_t* value){
  if (state == nullptr || value == nullptr) {
    OC_DEBUG("[%s]=false.control=%s", __func__, control_string);
    return false;
  }

  auto controller = GetRadarController(radar);
  if (controller == nullptr){
    OC_DEBUG("[%s]=false.control=%s", __func__, control_string);
    return false;
  }

  ControlType control_enum = ControlTypeStringToEnum(control_string);
  bool ret = controller->GetControlValue(control_enum, *((RadarPlugin::RadarControlState*)state), *value);
  if (!ret)
  {
    OC_DEBUG("[%s]=false.control=%s", __func__, control_string);
    return false;
  }
  OC_DEBUG("[%s]=true.control=%s.state=%d,value=%d.", __func__, control_string, *state, *value);
  return true;
}

void radar_set_position(const RadarPosition* pos) {
  OC_DEBUG("[%s] lat=%.7f,lon=%.7f,cog=%.1f,sog=%.1f,heading=%.1f,fix_time=%d,sats=%d", __func__, pos->lat,pos->lon,pos->cog,pos->sog,pos->heading,pos->timestamp,pos->sats);

  PlugIn_Position_Fix_Ex fix = {};

  fix.Lat = pos->lat;
  fix.Lon = pos->lon;
  fix.Cog = pos->cog;
  fix.Sog = pos->sog;
  fix.Var = 0;
  fix.Hdm = pos->heading;
  fix.Hdt = pos->heading;
  fix.FixTime = pos->timestamp; //now in seconds.
  fix.nSats = pos->sats;

  auto plugin = GetRadarPlugin();
  if (plugin)
    plugin->SetPositionFixEx(fix);
}

bool radar_set_guardzone_state(uint8_t radar, uint8_t zone, int state){ 
  //enabling guard zone. [This actually just enables the ALARM for the respective guardzone]
  //handle set guardzone? See Controls Dialog setGuardZoneVisibility? / ShowGuardZone?
  auto info = GetRadarInfo(radar);
  if (info == nullptr)
  {
    return false;
  }
  auto m_guard_zone = info->m_guard_zone[zone];
  m_guard_zone->m_show_time = time(0);
  m_guard_zone->SetAlarmOn(state);
  return true;
}

bool radar_set_guardzone_arpa(uint8_t radar, uint8_t zone, int state){ 
  //enabling guard zone. [This actually just enables the ALARM for the respective guardzone]
  //handle set guardzone? See Controls Dialog setGuardZoneVisibility? / ShowGuardZone?
  auto info = GetRadarInfo(radar);
  if (info == nullptr)
  {
    return false;
  }
  auto m_guard_zone = info->m_guard_zone[zone];
  m_guard_zone->m_show_time = time(0);
  m_guard_zone->SetArpaOn(state);
  return true;
}

bool radar_set_guardzone_type(uint8_t radar, uint8_t zone, int type){
  auto info = GetRadarInfo(radar); //hopefully returns a RadarInfo? RadarInfo then contains 
  if (info == nullptr)
  {
    return false;
  }
  auto m_guard_zone = info->m_guard_zone[zone]; // do i need auto's here??
  m_guard_zone->m_show_time = time(0);
  m_guard_zone->SetType((RadarPlugin::GuardZoneType)type); //mode should be 0 or 1 for arc / circle -> based ont he GuardZoneType enum
  return true;
}


bool radar_set_guardzone_define(uint8_t radar, uint8_t zone, int* defs){ //change this defintion.
  //unsure how they pick which radar | zone the below settings apply to... As it's done through the selection of the menu.
  
  //taken from on range X click's 
  //assume the array of range is in meters.
  auto info = GetRadarInfo(radar); //At this point this is a RadarInfo? -> It's ginvg me a RadarControl not RadarInfo
  if (info == nullptr)
  {
    return false;
  }

  auto m_guard_zone = info->m_guard_zone[zone];
  m_guard_zone->m_show_time = time(0);

  int inner = defs[0];
  int outer = defs[1];
  int start_bearing = defs[2];
  int end_bearing = defs[3];

  MOD_DEGREES(start_bearing);
  MOD_DEGREES(end_bearing);
  while (start_bearing < 0) {
    start_bearing += 360;
  }
  while (end_bearing < 0){
    end_bearing += 360;
  }

  m_guard_zone->SetInnerRange(inner);
  m_guard_zone->SetOuterRange(outer);
  m_guard_zone->SetStartBearing(start_bearing);
  m_guard_zone->SetEndBearing(end_bearing);
  return true;
}

GuardZoneStatus radar_get_guardzone_define(uint8_t radar)
{
  struct GuardZoneStatus pkt = {};
  auto info = GetRadarInfo(radar);
  if (info == nullptr){
    return pkt;
  }

  auto gz1 = info->m_guard_zone[0];
  auto gz2 = info->m_guard_zone[1];
  pkt.gz1_inner = gz1->m_inner_range;
  pkt.gz1_outer = gz1->m_outer_range;
  pkt.gz1_start = gz1->m_start_bearing;
  pkt.gz1_end = gz1->m_end_bearing;
  pkt.gz2_inner = gz2->m_inner_range;
  pkt.gz2_outer = gz2->m_outer_range;
  pkt.gz2_start = gz2->m_start_bearing;
  pkt.gz2_end = gz2->m_end_bearing;
  return pkt;
}

GuardZoneStatus radar_get_guardzone_type(uint8_t radar)
{
  struct GuardZoneStatus pkt = {};
  auto info = GetRadarInfo(radar);
  if (info == nullptr){
    return pkt;
  }
  auto gz1 = info->m_guard_zone[0];
  auto gz2 = info->m_guard_zone[1];
  pkt.gz1_type = gz1->m_type;
  pkt.gz2_type = gz2->m_type;
  return pkt;
}

GuardZoneStatus radar_get_guardzone_state(uint8_t radar)
{
  struct GuardZoneStatus pkt = {};
  auto info = GetRadarInfo(radar);
  if (info == nullptr){
    return pkt;
  }
  auto gz1 = info->m_guard_zone[0];
  auto gz2 = info->m_guard_zone[1];
  pkt.gz1_state = ((gz1->m_alarm_on << 1) | (gz1->m_arpa_on << 2));
  pkt.gz2_state = ((gz2->m_alarm_on << 1) | (gz2->m_arpa_on << 2)); //ask matt. 

  return pkt;
}

RadarControlStatus radar_get_control_status(uint8_t radar) {
  auto controller = GetRadarController(radar);
  auto info = GetRadarInfo(radar);
  struct RadarControlStatus pkt = {};

  if (info == nullptr || controller == nullptr){
    return pkt;
  }
  
  //I believe the getstates return an enum....?
  pkt.state = radar_get_state(radar);
  pkt.range = radar_get_range(radar);
  pkt.gain = info->m_gain.GetValue(); 
  pkt.sea = info->m_sea.GetValue();
  pkt.rain = info->m_rain.GetValue();
  pkt.auto_gain = info->m_gain.GetState();
  pkt.auto_sea = info->m_sea.GetState(); 
  pkt.auto_rain = info->m_rain.GetState();
  pkt.target_trails = info->m_target_trails.GetValue();
  pkt.target_boost =info->m_target_boost.GetValue();
  pkt.target_expansion = (info->m_target_expansion.GetValue() > 0) ? 1 : 0;  //currently a bug in the target_sepation interface that save value "on" as "high"
  pkt.target_separation = info->m_target_separation.GetValue();
  pkt.doppler = info->m_doppler.GetValue();
  pkt.scan_speed = info->m_scan_speed.GetValue();
  pkt.noise_rejection = info->m_noise_rejection.GetValue();
  return pkt;
}

bool check_guardzone_alarm(uint8_t radar){
  auto info = GetRadarInfo(radar);
  if (info)
    if (info->m_pi)
      if (info->m_pi->m_guard_bogey_seen) //doesn't check which zone triggered
        //todo findout why this causes segfaults unpredictably. (only noticed them while debugging....)
        return true;
  return false;
}

GuardZoneContactReport radar_get_guardzone_status(uint8_t radar)
{
  struct GuardZoneContactReport pkt = {};
  auto info = GetRadarInfo(radar);
  auto plugin = GetRadarPlugin();
  if (info == nullptr || plugin == nullptr){
    return pkt;
  }

  GeoPosition gps;
	info->GetRadarPosition(&gps);
  pkt.info_time = time(0); //should be the current time.
	//pkt.init_time = alarm;//time of first contact. 
	pkt.sensorid = 0; //radar a vs radar b
	pkt.contactid = 0; //increment in get gz state?
	pkt.our_lat = gps.lat;//todo, pull/update from mavlink message case.
	pkt.our_lon = gps.lon;
	//contact_info.our_hdg = info->m_true_heading_info; //todo, this is the wrong heading. :(

  return pkt;
}

bool radar_marpa_aquire(uint8_t radar, int bearing, int range){
  auto plugin = GetRadarPlugin();
  auto info = GetRadarInfo(radar); 
  if (info == nullptr || plugin == nullptr){
    return false;
  }
  Polar pol;
  pol.angle = bearing;
  pol.r = range; //hopefully this works okay?
  pol.time = wxGetUTCTimeMillis(); //now.

  GeoPosition gps;
	info->GetRadarPosition(&gps);
  ExtendedPosition curr;
  curr.pos = gps;

  ExtendedPosition target_pos = RadarPolar2Pos(radar, pol, curr);
  info->m_arpa->AcquireNewMARPATarget(target_pos); //should just aquire the target.
  return true;
}

bool radar_marpa_delete(uint8_t radar, int bearing, int range){
  auto info = GetRadarInfo(radar); 
  if (info == nullptr){
    return false;
  }
  Polar pol;
  pol.angle = bearing;
  pol.r = range; //hopefully this works okay?
  pol.time = wxGetUTCTimeMillis(); //now.

  GeoPosition gps;
	info->GetRadarPosition(&gps);
  ExtendedPosition curr;
  curr.pos = gps;

  ExtendedPosition target_pos = RadarPolar2Pos(radar, pol, curr);
  info->m_arpa->DeleteTarget(target_pos); //should just aquire the target.
  return true;
}

bool radar_marpa_delete_all(uint8_t radar){
  auto info = GetRadarInfo(radar); 
  if (info == nullptr){
    return false;
  }
  info->m_arpa->DeleteAllTargets(); //should just delete all targets
  return true;
}


//todo get target list function
ARPAContactReport radar_get_arpa_contact_report(uint8_t radar, int i){
  //fill in the data for the report 
  ARPAContactReport pkt = {};
  auto info = GetRadarInfo(radar);
  if (info == nullptr){
    return pkt;
  }

  if (i >= 100) //incorrect value.
    return pkt; 

  //below is invalid use of incomplete class?
  auto target = info->m_arpa->m_targets[i]; //m_targets was originally private
  if (target == nullptr){
    return pkt;
  }
  //we now have a bunch of info from target -> X

  //not sure if we have exposed enough of the marpa info, might need to make more bits public or move this to inside RadarMarpa
  pkt.sensorid = radar; //0,1 A,B
  
  if (target->m_automatic){
  //todo define this enum somewhere. {GuardZone, Arpa, Marpa}
    pkt.sensortype = 12; //SENSOR_TYPE_RADAR_ARPA; //true for ARPA
  } else {
    pkt.sensortype = 13; //SENSOR_TYPE_RADAR_MARPA; //false for MARPA
  }
  pkt.contactid = target->m_target_id;
  pkt.init_time = time(0) - (int) target->m_refresh.GetValue(); //probably casting wrong.
  pkt.info_time = time(0);
  pkt.lat = target->m_position.pos.lat;
  pkt.lon = target->m_position.pos.lon;
  pkt.sog = target->m_speed_kn;
  pkt.cog = target->m_course;
  GeoPosition gps;
	info->GetRadarPosition(&gps);
  ExtendedPosition curr;
  curr.pos = gps;

  Polar pol = RadarPos2Polar(radar, target->m_position, curr);
  pkt.range = pol.r / info->m_pixels_per_meter / 1852.; //eqns from PassARPAtoOCPN
  pkt.bearing = pol.angle * 360. / info->m_spokes;
  
  pkt.phase = target->m_status; //a target_status (Q, T, L) for acquring active lost.
  pkt.our_lat = gps.lat; 
  pkt.our_lon = gps.lon;

  return pkt;
}

ExtendedPosition RadarPolar2Pos(uint8_t radar, const RadarPlugin::Polar& pol, const ExtendedPosition& own_ship) {
  //just simplifying stuff by moving this here and getting the m_ri through the getter function.

  // The "own_ship" in the function call can be the position at an earlier time than the current position
  // converts in a radar image angular data r ( 0 - max_spoke_len ) and angle (0 - max_spokes) to position (lat, lon)
  // based on the own ship position own_ship
  ExtendedPosition pos;
  auto m_ri = GetRadarInfo(radar);
  if (m_ri == nullptr){
    return pos; //empty.
  }

  pos.pos.lat = own_ship.pos.lat + ((double)pol.r / m_ri->m_pixels_per_meter)  // Scale to fraction of distance from radar
                                       * cos(deg2rad(SCALE_SPOKES_TO_DEGREES(pol.angle))) / 60. / 1852.;
  pos.pos.lon = own_ship.pos.lon + ((double)pol.r / m_ri->m_pixels_per_meter)  // Scale to fraction of distance to radar
                                       * sin(deg2rad(SCALE_SPOKES_TO_DEGREES(pol.angle))) / cos(deg2rad(own_ship.pos.lat)) / 60. /
                                       1852.;
  return pos;
}

Polar RadarPos2Polar(uint8_t radar, const ExtendedPosition& p, const ExtendedPosition& own_ship) {
   //just simplifying stuff by moving this here and getting the m_ri through the getter function.

  Polar pol;
  auto m_ri = GetRadarInfo(radar);
  if (m_ri == nullptr){
    return pol;
  }

  double dif_lat = p.pos.lat;
  dif_lat -= own_ship.pos.lat;
  double dif_lon = (p.pos.lon - own_ship.pos.lon) * cos(deg2rad(own_ship.pos.lat));
  pol.r = (int)(sqrt(dif_lat * dif_lat + dif_lon * dif_lon) * 60. * 1852. * m_ri->m_pixels_per_meter + 1);
  pol.angle = (int)((atan2(dif_lon, dif_lat)) * (double)m_ri->m_spokes / (2. * PI) + 1);  // + 1 to minimize rounding errors
  if (pol.angle < 0) pol.angle += m_ri->m_spokes;
  return pol;
}