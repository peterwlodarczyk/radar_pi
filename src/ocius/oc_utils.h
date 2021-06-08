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
#endif
