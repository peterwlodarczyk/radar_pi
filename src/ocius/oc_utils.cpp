#ifdef _WIN32
#else
#include <sys/time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#include <stdarg.h>
#include <string.h>

#include <fstream>
#include <string>
#include <vector>
#include <chrono>

#include "oc_utils.h"

using namespace std;

static bool Enabled = true;
string MakeLocalTimeStamp() {
  char achTemp[256];
  achTemp[0] = '\0';

#if defined(WIN32)
  //SYSTEMTIME st;
  //GetLocalTime(&st);
#else
  timeval ts;
  gettimeofday(&ts, 0);
  time_t t;
  time(&t);
  struct tm* tm = localtime(&t);
#endif

#if defined(WIN32)
  //sprintf(achTemp, "%04d-%02d-%02d %02d-%02d-%02d-%03d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond,
  //        st.wMilliseconds);
#else
  long msecs = ts.tv_usec / 1000;
  sprintf(achTemp, "%04d-%02d-%02d %02d-%02d-%02d-%03ld", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min,
          tm->tm_sec, msecs);
#endif
  return achTemp;
}

std::vector<uint8_t> JpegAppendComment(const std::vector<uint8_t>& input, const string& timestamp, const string& camera) {
    static const char achCom[3] = {(char)0xff, (char)0xfe, (char)0x00};
    static const uint8_t achEOI[3] = {0xff, 0xd9, 0x00};

    std::vector<uint8_t> output;
    OC_DEBUG("0x%2X,0x%2X", input[input.size() - 2], input[input.size() - 1]);
    OC_DEBUG("0x%2X,0x%2X", achEOI[0], achEOI[1]);
    if (input.size() < 2 || input[input.size() - 2] != achEOI[0] || input[input.size() - 1] != achEOI[1]) return output;

    output = input;

    string contents = string(achCom) + timestamp + ';' + camera + ';' + "OK;" + "<XMLData/>";
    vector<uint8_t> bytes(contents.begin(), contents.end());

    output.insert(output.end() - 2, bytes.begin(), bytes.end());
    return output;
}

  std::vector<uint8_t> readfile(const char* filename) {
    std::ifstream stream(filename, std::ios::in | std::ios::binary);
    return std::vector<uint8_t>((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
  }

extern std::string g_OciusLogFilename;

static void LogWrite(const char* t, const char* str) {
    FILE* f = fopen(g_OciusLogFilename.c_str(), "a");
    if (f) {
      fprintf(f, "%s %s", MakeLocalTimeStamp().c_str(), str);
      size_t len = strlen(str);
      if (len > 0 && str[len - 1] != '\n') fprintf(f, "%s", "\n");
      fflush(f);
      fclose(f);
      f = 0;
    }
  }

void OC_DEBUG(const char* format, ...) {
  if (!Enabled) return;

  char achDebug[4096];
  va_list args;
  va_start(args, format);
  vsnprintf(achDebug, sizeof(achDebug), format, args);
  va_end(args);
  LogWrite("DEBUG", achDebug);
}

bool CreateFileWithPermissions(const char* filename, int mode) {
  bool ret = false;
#ifndef WIN32
  if (access(filename, 0)) {
    mode_t old_mask = umask(0);
    int hf = open(filename, O_RDWR | O_CREAT, mode);
    if (hf) {
        close(hf);
        ret = true;
    }
    umask(old_mask);
  }
#endif
  return ret;
}

double TimeInSeconds()
{
    return std::chrono::duration_cast< std::chrono::microseconds >(std::chrono::system_clock::now().time_since_epoch()).count() / 1000000.0;
}

uint64_t TimeInMicros()
{
    return std::chrono::duration_cast< std::chrono::microseconds >(std::chrono::system_clock::now().time_since_epoch()).count();
}

TimerT Timer0("Timer0");
void LogTimers()
{
  OC_DEBUG("Timers-----------------");
  for ( auto& t : Timers())
     if (t.should_log_)
      OC_DEBUG("%s", to_string(t).c_str());
  if (Timer0.should_log_)
    OC_DEBUG("%s", to_string(Timer0).c_str());
  OC_DEBUG("-----------------");
}

std::string to_string(const TimerT& t)
{
  char buf[500];
  sprintf(buf, "[%25.25s]:Load=%5.1f,Rate=%6.1f,Last=%6.4f,Mean=%6.4f,Min=%6.4f,Max=%6.4f,Count=%5u,Accumulated=%6.1f", 
    t.GetName().c_str(), 
    t.GetLoad(), 
    t.GetRate(), 
    t.GetPeriod(), 
    t.GetMean(), 
    t.GetMin(), 
    t.GetMax(), 
    t.count_, 
    t.GetAccumulated());
  return std::string(buf);
}

void TimerT::Stop()
{
  uint64_t now = TimeInMicros();
  stop_ = now;
  //OC_DEBUG("Start=%llu", stop_);
  period_ = stop_ - start_;
  //OC_DEBUG("Period=%llu", period_);
  if (period_ < min_)
    min_ = period_;
  if (period_ > max_)
    max_ = period_;
  sum_ += period_;
  ++count_;

  // Update the percentage rate.
  if (update_time_ == 0)
  {
    update_time_ = now;
    sum0_ = sum_;
    count0_ = count_;
  }
  else
  {
    int time_diff = now - update_time_;
    if (time_diff > 5000000)
    {
      load_ = (sum_ - sum0_) * 100.0 / time_diff;
      update_time_ = now;
      rate_ = (count_ - count0_) * 1000000.0 / time_diff;

      sum0_ = sum_;
      count0_ = count_;
    }
  }
}

std::vector<TimerT>& Timers()
{
  static std::vector<TimerT> Timers_;
  if (Timers_.size() == 0)
  {
    // MyFrame::OnRenderTimer
    Timers_.push_back(TimerT("PlugInManager::RenderAllGLCanvasOverlayPlugIns", true));  // 0
    // RadarCanvas::Render
    Timers_.push_back(TimerT("RadarCanvas::Render0", true));  // 1
    Timers_.push_back(TimerT("RadarCanvas::Render1", true));  // 2
    Timers_.push_back(TimerT("RadarCanvas::Render2", true));  // 3
    Timers_.push_back(TimerT("RadarCanvas::Render3", true));  // 4
    Timers_.push_back(TimerT("RadarCanvas::Render4", true));  // 5
    Timers_.push_back(TimerT("RadarCanvas::Render5", true));  // 6
    Timers_.push_back(TimerT("RadarCanvas::Render6", true));  // 7
    Timers_.push_back(TimerT("radar_pi::RenderGLOverlayMultiCanvas", true)); // 8
    Timers_.push_back(TimerT("RadarArpa::RefreshArpaTargets", true)); // 9
    Timers_.push_back(TimerT("RadarDrawVertex::DrawRadarOverlayImage")); // 10
    Timers_.push_back(TimerT("RadarDrawVertex::DrawRadarPanelImage")); // 11
    Timers_.push_back(TimerT("OciusDumpVertexImage", true)); // 12
    // NavicoReceive::ProcessFrame
    Timers_.push_back(TimerT("NavicoReceive::ProcessFrame")); // 13
    Timers_.push_back(TimerT("RadarInfo::ProcessRadarSpoke")); // 14
  }
  return Timers_;
}

TimerT& Timer(int timer)
{
  return Timers()[timer];
}
