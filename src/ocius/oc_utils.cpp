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

  void CreateFileWithPermissions(const char* filename, int mode) {
#ifndef WIN32
    if (access(filename, 0)) {
      mode_t old_mask = umask(0);
      int hf = open(filename, O_RDWR | O_CREAT, 0666);
      if (hf) close(hf);
      umask(old_mask);
    }
#endif
  return;
}
