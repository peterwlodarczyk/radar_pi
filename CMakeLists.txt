##---------------------------------------------------------------------------
## Plagiarized by: Peter Wlodarczyk for WINDOWS
##
## Author:      bdbcat aka. dsr (Dave Register)
## Copyright:
## License:     wxWidgets License
##---------------------------------------------------------------------------

# define minimum cmake version
CMAKE_MINIMUM_REQUIRED(VERSION 2.6.2)

option(RADAR_EXE "Build radar exe" OFF)
option(OC_DP "Build ocius daemon proc" ON)
option(OPENCPN_PLUGIN "OpenCPN Plugin" OFF)

PROJECT(radar_pi)
SET(PACKAGE_NAME radar_pi)
SET(VERBOSE_NAME Radar)
SET(TITLE_NAME Radar)
SET(CPACK_PACKAGE_CONTACT "peter@ocius.com.au")
#SET(CMAKE_BUILD_TYPE Debug)
SET(VERSION_MAJOR "5")
SET(VERSION_MINOR "0")
SET(VERSION_PATCH "5")
SET(VERSION_DATE "2020-05-15")
SET(OCPN_MIN_VERSION "ov50")

IF(MSVC)
SET(wxWidgets_ROOT_DIR "C:/dev/libs/wxWidgets_src")
SET(wxWidgets_LIB_DIR "C:/dev/libs/wxWidgets_src/lib/vc_dll")
SET(wxWidgets_LIBRARIES "C:/dev/libs/wxWidgets_src/lib")
SET(wxWidgets_INCLUDE_DIRS "C:/dev/libs/wxWidgets_src/include/wx")
set(wxWidgets_INCLUDE_DIRS "C:/dev/libs/wxWidgets_src")
set(wxWidgets_LIBRARIES "C:/dev/libs/wxWidgets_src/lib/vc_dll")
set(wxWidgets_FIND_COMPONENTS net xml html adv aui core base gl)
ENDIF()

INCLUDE("cmake/PluginConfigure.cmake")

# For convenience we define the sources as a variable. You can add
# header files and cpp/c files and CMake will sort them out

SET(SRC_NAVICO
            src/navico/br24type.h
            src/navico/br3gtype.h
            src/navico/br4gatype.h
            src/navico/br4gbtype.h
            src/navico/haloatype.h
            src/navico/halobtype.h
            src/navico/NavicoCommon.h
            src/navico/NavicoReceive.cpp
            src/navico/NavicoReceive.h
            src/navico/NavicoControl.cpp
            src/navico/NavicoControl.h
            src/navico/NavicoControlSet.h
            src/navico/NavicoControlsDialog.cpp
            src/navico/NavicoControlsDialog.h
            src/navico/NavicoLocate.cpp
            src/navico/NavicoLocate.h
            src/navico/NavicoRadarInfo.h
)

SET(SRC_OCIUS
            src/ocius/oc_utils.cpp
            src/ocius/oc_utils.h
            src/ocius/oc_radar_interface.cpp
            src/ocius/oc_radar_interface.h
)

SET(SRC_RADAR
            src/ControlsDialog.cpp
            src/ControlsDialog.h
            src/GuardZone.cpp
            src/GuardZone.h
            src/GuardZoneBogey.cpp
            src/GuardZoneBogey.h
            src/Kalman.cpp
            src/Kalman.h
            src/Matrix.h
            src/MessageBox.cpp
            src/MessageBox.h
            src/OptionsDialog.cpp
            src/OptionsDialog.h
            src/RadarCanvas.cpp
            src/RadarCanvas.h
            src/RadarControl.h
            src/RadarControlItem.h
            src/RadarDraw.cpp
            src/RadarDraw.h
            src/RadarDrawShader.cpp
            src/RadarDrawShader.h
            src/RadarDrawVertex.cpp
            src/RadarDrawVertex.h
            src/RadarFactory.cpp
            src/RadarFactory.h
            src/RadarInfo.cpp
            src/RadarInfo.h
            src/RadarMarpa.cpp
            src/RadarMarpa.h
            src/radar_pi.cpp
            src/radar_pi.h
            src/RadarPanel.cpp
            src/RadarPanel.h
            src/RadarReceive.h
            src/RadarType.h
            src/SelectDialog.cpp
            src/SelectDialog.h
            src/SoftwareControlSet.h
            src/TextureFont.cpp
            src/TextureFont.h
            src/TrailBuffer.h
            src/TrailBuffer.cpp
            src/ControlsDialog.cpp
            src/ControlsDialog.h
            src/drawutil.cpp
            src/drawutil.h
            src/icons.cpp
            src/icons.h
            src/pi_common.h
            src/shaderutil.cpp
            src/shaderutil.h
            src/socketutil.cpp
            src/socketutil.h

)

SET(SRC_PLUGIN
)            

SET(SRC_RADAR_EXE
            src/exe/pluginmanager.cpp
            src/exe/radar_main.cpp
)            

SET(SRC_OC_DP
            ../oc_usv/oc_common/oc_args.cpp
            ../oc_usv/oc_common/oc_buffer.cpp
            ../oc_usv/oc_common/oc_conf.cpp
            ../oc_usv/oc_common/oc_dirs.cpp
            ../oc_usv/oc_common/oc_fileio.cpp
            ../oc_usv/oc_common/oc_lock.cpp
            ../oc_usv/oc_common/oc_log4_oc.cpp
            ../oc_usv/oc_common/oc_noservconf.cpp
            ../oc_usv/oc_common/oc_pipe.cpp
            ../oc_usv/oc_common/oc_proc_helpers.cpp
            ../oc_usv/oc_common/oc_status.cpp
            ../oc_usv/oc_common/oc_utils.cpp
            ../oc_usv/oc_common/oc_xmlutils.cpp
            ../oc_usv/oc_common/wininc.cpp
            ../oc_usv/oc_dp_radar/oc_dp_radar.cpp
            src/exe/pluginmanager.cpp
            src/exe/radar_main.cpp
)

SET(SRC_NMEA0183
        src/nmea0183/nmea0183.cpp
        src/nmea0183/nmea0183.hpp
        src/nmea0183/Response.hpp
        src/nmea0183/response.cpp
        src/nmea0183/Sentence.hpp
        src/nmea0183/sentence.cpp
        src/nmea0183/talkerid.cpp
        src/nmea0183/hexvalue.cpp
        src/nmea0183/expid.cpp
        src/nmea0183/hdt.hpp
        src/nmea0183/hdt.cpp
        src/nmea0183/hdg.hpp
        src/nmea0183/hdg.cpp
        src/nmea0183/hdm.hpp
        src/nmea0183/hdm.cpp
)
SET(SRC_JSON
        src/wxJSON/jsonreader.cpp
        src/wxJSON/jsonval.cpp
        # We don't use jsonwriter.cpp yet ...
)

#find_package(OpenGL)

INCLUDE_DIRECTORIES(src/nmea0183)
INCLUDE_DIRECTORIES(src/wxJSON)
INCLUDE_DIRECTORIES(src/exe)
INCLUDE_DIRECTORIES(src)

IF (RADAR_EXE)
  MESSAGE(STATUS "Radar Executable")
  ADD_EXECUTABLE(${PACKAGE_NAME} ${SRC_RADAR_EXE} ${SRC_RADAR} ${SRC_NMEA0183} ${SRC_JSON} ${SRC_OCIUS} ${SRC_NAVICO})
  ADD_DEFINITIONS(-DRADAR_EXE)
  TARGET_LINK_LIBRARIES(${PACKAGE_NAME} ${wxWidgets_LIBRARIES} ${OPENGL_LIBRARY})
ELSEIF (OC_DP)
  FIND_PACKAGE(LibXml2 REQUIRED)
  MESSAGE(STATUS "found libxml ${LIBXML2}")
  find_library(LIBLOG4 liblog4cplus.so HINTS "/usr/lib")
  find_library(LIBXML2 libxml2.so HINTS "/usr/lib")

  INCLUDE_DIRECTORIES(${LIBXML2_INCLUDE_DIR})
  INCLUDE_DIRECTORIES("../oc_usv/oc_common")
  
  MESSAGE(STATUS "ocius daemon")
  #ADD_EXECUTABLE(${PACKAGE_NAME} ${SRC_OC_DP})
  ADD_EXECUTABLE(${PACKAGE_NAME} ${SRC_OC_DP} ${SRC_RADAR} ${SRC_NMEA0183} ${SRC_JSON} ${SRC_OCIUS} ${SRC_NAVICO})
  ADD_DEFINITIONS(-DOC_DP)
  TARGET_LINK_LIBRARIES(${PACKAGE_NAME} 
    ${wxWidgets_LIBRARIES} 
    ${OPENGL_LIBRARY}
	${LIBXML2}
	${LIBLOG4}
	-ljpeg
	-lz
	-lpthread
  )
ELSE()
  MESSAGE(STATUS "OpenCPN Plugin")
  IF (OPENCPN_PLUGIN)
    ADD_DEFINITIONS(-DOPENCPN_PLUGIN)
  ELSE()
    ADD_DEFINITIONS(-DOCIUS_PLUGIN)
  ENDIF()
  ADD_LIBRARY(${PACKAGE_NAME} SHARED ${SRC_PLUGIN} ${SRC_RADAR} ${SRC_NMEA0183} ${SRC_JSON} ${SRC_OCIUS} ${SRC_NAVICO})
  INCLUDE("cmake/PluginInstall.cmake")
  INCLUDE("cmake/PluginLocalization.cmake")
  INCLUDE("cmake/PluginPackage.cmake")
ENDIF()
