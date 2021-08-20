#ifndef __OC_UTILS_H
#define __OC_UTILS_H
#include <stdint.h>

#include <string>
#include <vector>

void OC_DEBUG(const char* format, ...);
void OC_TRACE(const char* format, ...);

std::string MakeLocalTimeStamp();
std::vector<uint8_t> JpegAppendComment(const std::vector<uint8_t>& input, const std::string& timestamp, const std::string& camera);
bool CreateFileWithPermissions(const char* filename, int mode);
std::vector<uint8_t> readfile(const char* filename);
double TimeInSeconds();
uint64_t TimeInMicros();

class ProfilerT
{
  public:
    ProfilerT(const char* name = "", bool log_entry_exit = false, bool should_log = true)
        : name_(name),
        should_log_(should_log),
        log_entry_exit_(log_entry_exit)
    {
    }
    void Start()
    {
      if (Enabled)
        start_ = TimeInMicros();
    }
    void Stop();

    static void Enable(bool enable);

    const std::string& GetName() const { return name_; }
    double GetPeriod() const { return period_ / 1000000.0; }
    double GetAccumulated() const { return sum_ / 1000000.0; }
    double GetMin() const { return min_ / 1000000.0; }
    double GetMax() const { return max_ / 1000000.0; }
    double GetMean() const 
    {
      if (count_ == 0)
        return 0;
      else
        return sum_ / count_ / 1000000.0;
    }
    double GetLoad() const { return load_; }
    double GetRate() const { return rate_; }

    std::string name_;
    uint64_t period_ = 0;
    uint64_t min_ = 10000000000000;
    uint64_t max_ = 0;
    uint64_t sum_ = 0;
    uint64_t sum0_ = 0;
    uint64_t count0_ = 0;
    uint32_t count_ = 0;

    uint64_t start_ = 0;
    uint64_t stop_ = 0;
    uint64_t update_time_ = 0;
    double load_ = 0;
    double rate_ = 0;
    bool should_log_ = true;
    bool log_entry_exit_ = false;
    static bool Enabled;
};

void LogProfilers();

#define PLUGINMANAGER_RENDERALLGLCANVASOVERLAYPLUGINS 0
#define RADARCANVAS_RENDER0 1
#define RADARCANVAS_RENDER1 2
#define RADARCANVAS_RENDER2 3
#define RADARCANVAS_RENDER3 4
#define RADARCANVAS_RENDER4 5
#define RADARCANVAS_RENDER5 6
#define RADARCANVAS_RENDER6 7
#define RADARCANVAS_RENDER7 8
#define RADAR_PI_RENDERGLOVERLAYMULTICANVAS 9
#define RADARARPA_REFRESHARPATARGETS 10
#define RADARDRAWVERTEX_DRAWRADAROVERLAYIMAGE 11
#define RADARDRAWVERTEX_DRAWRADARPANELIMAGE 12
#define OCIUSDUMPVERTEXIMAGE 13
#define NAVICORECEIVE_PROCESSFRAME 14
#define RADARINFO_PROCESSRADARSPOKE 15

std::vector<ProfilerT>& Profilers();
ProfilerT& Profiler(int timer);

class ProfilerGuardT
{
  public:
    ProfilerGuardT(int t)
     : t_(Profiler(t))
    {
      if (ProfilerT::Enabled)
      {
        if (t_.log_entry_exit_)
          OC_TRACE("[%s]>>", t_.name_.c_str());
        t_.Start();
      }
    }

    ProfilerGuardT(ProfilerT& t)
     : t_(t)
    {
      if (ProfilerT::Enabled)
      {
        if (t_.log_entry_exit_)
          OC_TRACE("[%s]>>", t_.name_.c_str());
        t_.Start();
      }
    }

    ~ProfilerGuardT()
    {
      if (ProfilerT::Enabled)
      {
        t_.Stop();
        if (t_.log_entry_exit_)
          OC_TRACE("[%s]<<", t_.name_.c_str());
      }
    }

  private:
    ProfilerT& t_;
};

std::string to_string(const ProfilerT& t);

#endif
