#ifndef __RADAR_LIB_H
#define __RADAR_LIB_H

#ifndef DECL_IMPEXP
#ifdef RADAR_LIB
#ifdef WIN32
#define DECL_IMPEXP __declspec(dllexport)
#else
#ifdef __GNUC__
#define DECL_IMPEXP __attribute__((visibility("default")))
#else
#define DECL_IMPEXP
#endif
#endif
#else
#ifdef WIN32
#define DECL_IMPEXP __declspec(dllimport)
#else
#define DECL_IMPEXP
#endif
#endif
#endif
#include <stdint.h>

enum RadarStatus {
  OC_RADAR_STATUS_INACTIVE=0,
  OC_RADAR_STATUS_HEALTHY=1,
  // OC_RADAR_STATUS_WARN=2,
  // OC_RADAR_STATUS_DEGRADED=3,
  OC_RADAR_STATUS_FAULT=4,
};

extern "C" DECL_IMPEXP bool radar_start(const char* configFilename, const char* logDir, const char* liveDir);
extern "C" DECL_IMPEXP void radar_stop();
extern "C" DECL_IMPEXP ::RadarStatus radar_get_status();
extern "C" DECL_IMPEXP bool radar_set_enable(uint8_t radar, bool enable);
extern "C" DECL_IMPEXP bool radar_get_enable(uint8_t radar);

enum RadarState {
  OC_RADAR_OFF,
  OC_RADAR_STANDBY,
  OC_RADAR_WARMING_UP,
  OC_RADAR_TIMED_IDLE,
  OC_RADAR_STOPPING,
  OC_RADAR_SPINNING_DOWN,
  OC_RADAR_STARTING,
  OC_RADAR_SPINNING_UP,
  OC_RADAR_TRANSMIT
};

extern "C" DECL_IMPEXP ::RadarState radar_get_state(uint8_t radar);
extern "C" DECL_IMPEXP bool radar_set_tx(uint8_t radar, bool on);
extern "C" DECL_IMPEXP bool radar_get_tx(uint8_t radar);
extern "C" DECL_IMPEXP double radar_set_range(uint8_t radar, double range);  // range in metres. Auto??a
extern "C" DECL_IMPEXP double radar_get_range(uint8_t radar);
extern "C" DECL_IMPEXP uint32_t radar_get_activity_count(); // Across all radars. Should 
extern "C" DECL_IMPEXP uint32_t radar_get_spoke_count(uint8_t radar); // the number of spoked received
extern "C" DECL_IMPEXP uint32_t radar_get_image_count(uint8_t radar); // the number of time a radar image has been successfully writen

extern "C" DECL_IMPEXP bool radar_set_guardzone_state(uint8_t radar, uint8_t zone, int state);
extern "C" DECL_IMPEXP bool radar_set_guardzone_arpa(uint8_t radar, uint8_t zone, int state);
extern "C" DECL_IMPEXP bool radar_set_guardzone_type(uint8_t radar, uint8_t zone, int type);
extern "C" DECL_IMPEXP bool radar_set_guardzone_define(uint8_t radar, uint8_t zone, int* defs);
extern "C" DECL_IMPEXP bool radar_marpa_aquire(uint8_t radar, int bearing, int range);
extern "C" DECL_IMPEXP bool radar_marpa_delete(uint8_t radar, int bearing, int range);
extern "C" DECL_IMPEXP bool radar_marpa_delete_all(uint8_t radar);

extern "C" DECL_IMPEXP void radar_enable_profiling(bool enable);
extern "C" DECL_IMPEXP uint32_t radar_set_update_period(uint32_t period_ms);
extern "C" DECL_IMPEXP uint32_t radar_get_update_period();
extern "C" DECL_IMPEXP void radar_set_render_decimate(uint8_t radar, uint8_t decimate_rate); // Reduces the update_rate
extern "C" DECL_IMPEXP uint8_t radar_get_render_decimate(uint8_t radar);
extern "C" DECL_IMPEXP void radar_set_image_decimate(uint8_t radar, uint8_t decimate_rate); // Reduces the render rate
extern "C" DECL_IMPEXP uint8_t radar_get_image_decimate(uint8_t radar);

enum RadarControlState {
  OC_CS_OFF = -1,
  OC_CS_MANUAL = 0,
  OC_CS_AUTO_1,
  OC_CS_AUTO_2,
  OC_CS_AUTO_3,
  OC_CS_AUTO_4,
  OC_CS_AUTO_5,
  OC_CS_AUTO_6,
  OC_CS_AUTO_7,
  OC_CS_AUTO_8,
  OC_CS_AUTO_9
};

extern "C" DECL_IMPEXP bool radar_set_control(uint8_t radar, const char* control, ::RadarControlState state, int32_t value);
extern "C" DECL_IMPEXP bool radar_set_item_control(uint8_t radar, const char* control_string, ::RadarControlState state, int32_t value);
extern "C" DECL_IMPEXP bool radar_get_control(uint8_t radar, const char* control, ::RadarControlState* state, int32_t* value);
extern "C" DECL_IMPEXP bool radar_config_save();

struct RadarPosition {
  double lat; // degrees
  double lon; // degrees
  double cog; // degrees
  double sog; // m/s
  double heading; // degrees
  uint64_t timestamp; // microseconds since 1970 UTC
  int sats;
};
struct GuardZoneStatus {
  double gz1_inner; // meters
  double gz1_outer; // meters
  double gz1_start; // degrees
  double gz1_end; // degrees
  int gz1_state; // on/off
  int gz1_type; //GuardZoneType enum
  int gz1_threshold;
  double gz2_inner; // meters
  double gz2_outer; // meters
  double gz2_start; // degrees
  double gz2_end; // degrees
  int gz2_state; // on/off
  int gz2_type; //GuardZoneType enum
  int gz2_threshold;
};

struct RadarControlStatus {
  int range; //metres
  int state;
  int gain; //db
  int rain; //db
  int sea; //db
  int auto_gain; // degrees
  int auto_rain; // degrees
  int auto_sea; // on/off
  int target_trails;
  int target_boost;
  int target_expansion;
  int target_separation;
  int doppler;
  int scan_speed;
  int noise_rejection;
};
struct GuardZoneContactReport {
  int sensortype; //radar enum //actually needs to be 7 for radar
  int sensorid; //enum(GuardZone, ARPA, MARPA)
  int contactid; //increment when alarm goes true.
  int init_time; //start contact time.
  int info_time; //current time
  float our_lat; //our pos from mavlink messages.
  float our_lon; //our pos from mavlink messages.
  int our_hdg; //our pos from mavlink messages.
};

struct ARPAContactReport {
  int sensortype; //radar enum //actually needs to be 7 for radar
  int sensorid; //enum(GuardZone, ARPA, MARPA) ->  m_automatic
  int contactid; // [m_target_id]
  int init_time; //start contact time.
  int info_time; //current time
  //some of the data is here PassARPAtoOCPN
  float lat; //target pos m_position.lat
  float lon; //target pos m_position.lon
  float cog; //target cog [m_course]
  float sog; //target sog [m_speed_kn]
  float our_lat; //our pos from mavlink messages.
  float our_lon; //our pos from mavlink messages.
  int our_hdg; //our pos from mavlink messages.
  double bearing; //bearing = pol->angle * 360. / m_ri->m_spokes;
  double range; //dist = pol->r / m_ri->m_pixels_per_meter / 1852.
  int phase; //state of tracking.
};

extern "C" DECL_IMPEXP void radar_set_position(const RadarPosition* pos);
extern "C" DECL_IMPEXP GuardZoneStatus radar_get_guardzone_state(uint8_t radar);
extern "C" DECL_IMPEXP GuardZoneStatus radar_get_guardzone_type(uint8_t radar);
extern "C" DECL_IMPEXP GuardZoneStatus radar_get_guardzone_define(uint8_t radar);
extern "C" DECL_IMPEXP RadarControlStatus radar_get_control_status(uint8_t radar);
extern "C" DECL_IMPEXP GuardZoneContactReport radar_get_guardzone_status(uint8_t radar);
extern "C" DECL_IMPEXP ARPAContactReport radar_get_arpa_contact_report(uint8_t radar, int i);
extern "C" DECL_IMPEXP bool check_guardzone_alarm(uint8_t radar);

#endif
