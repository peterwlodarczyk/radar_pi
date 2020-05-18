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

extern "C" DECL_IMPEXP bool radar_start(const char* configFilename, const char* logDir, const char* liveDir);
extern "C" DECL_IMPEXP void radar_stop();
extern "C" DECL_IMPEXP bool radar_set_tx(int radar, bool on);
extern "C" DECL_IMPEXP bool radar_get_tx(int radar);
extern "C" DECL_IMPEXP int radar_set_range(int radar, int range);  // range in metres. Auto??a
extern "C" DECL_IMPEXP int radar_get_range(int radar); 
extern "C" DECL_IMPEXP int radar_set_control(int radar, const char* control, int value);
extern "C" DECL_IMPEXP int radar_get_control(int radar, const char* control);
struct RadarState {
  bool on;
};
extern "C" DECL_IMPEXP RadarState radar_get_state(int radar);

#endif
