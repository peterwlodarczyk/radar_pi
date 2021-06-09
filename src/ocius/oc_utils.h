#ifndef __OC_UTILS_H
#define __OC_UTILS_H
#include <stdint.h>

#include <string>
#include <vector>

#define OC_TRACE OC_DEBUG
void OC_DEBUG(const char* format, ...);

std::string MakeLocalTimeStamp();
std::vector<uint8_t> JpegAppendComment(const std::vector<uint8_t>& input, const std::string& timestamp, const std::string& camera);
bool CreateFileWithPermissions(const char* filename, int mode);
std::vector<uint8_t> readfile(const char* filename);
double TimeInSeconds();
uint64_t TimeInMicros();

class TimerT
{
  public:
    TimerT(const char* name = "")
        : name_(name)
    {
    }
    void Start()
    {
      start_ = TimeInMicros();
      //OC_DEBUG("Start=%llu", start_);
    }

    void Stop()
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
      }
      else
      {
        if ((now - update_time_) > 2000000)
        {
          percent_ = (sum_ - sum0_) * 100.0 / (now - update_time_);
          update_time_ = now;
          sum0_ = sum_;
        }
      }
    }

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

    std::string name_;
    uint64_t period_ = 0;
    uint64_t min_ = 10000000000000;
    uint64_t max_ = 0;
    uint64_t sum_ = 0;
    uint64_t sum0_ = 0;
    uint32_t count_ = 0;

    uint64_t start_ = 0;
    uint64_t stop_ = 0;
    uint64_t update_time_ = 0;
    double percent_ = 0;
};

void LogTimers();
std::vector<TimerT>& Timers();
extern TimerT Timer0;

class TimerGuardT
{
  public:
    TimerGuardT(TimerT& t)
     : t_(t)
    {
        t_.Start();
    }

    ~TimerGuardT()
    {
        t_.Stop();
    }

    TimerT& t_;
};

std::string to_string(const TimerT& t);

#endif
