#ifndef __OC_UTILS_H
#define __OC_UTILS_H
#include <string>
#include <vector>
#include <stdint.h>

#define OC_TRACE(...)
void OC_DEBUG(const char *format, ...);

std::string MakeLocalTimeStamp();
std::vector<uint8_t> JpegAppendComment(const std::vector<uint8_t>& input, const std::string& timestamp, const std::string& camera);

std::vector<uint8_t> readfile(const char* filename);

#endif
