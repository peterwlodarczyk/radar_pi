#include <time.h>

#include <chrono>
#include <fstream>
#include <string>

#include "oc_utils.h"
#include "oc_lock.h"
#include "pi_common.h"

using namespace std;
using namespace std::chrono;

extern std::string g_OciusLiveDir;

static system_clock::time_point next_update = system_clock::now();
static int oc_count = 0;

void OciusDumpVertexImage(int radar) {
  system_clock::time_point now = system_clock::now();
  if (now < next_update) 
    return;
  next_update = now + milliseconds(100);
  string name;
  if (radar == 1)
    name = string("radarb");
  else
    name = string("radara");

  OC_DEBUG("[OciusDumpVertexImage] %s:%d>>", name.c_str(), oc_count);
  ++oc_count;

  GLint viewport[4];
  glGetIntegerv(GL_VIEWPORT, viewport);

  int x = viewport[2];
  int y = viewport[3];
  int inputSize = x *y * 4;
  unsigned char *buffer = (unsigned char *)malloc(inputSize);
  memset(buffer, 0, inputSize);
  glReadPixels(0, 0, x, y, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
  int inputSum = 0;
  for (int i = 0; i < inputSize; ++i)
    inputSum += buffer[i];
  OC_TRACE("Vierport:%d,%d,%d,%d.inputSum=%d.\n", viewport[0], viewport[1], viewport[2], viewport[3], inputSum);

  int outputSize = x * y * 3;
  unsigned char *rgb = (unsigned char *)malloc(outputSize);
  unsigned char *alpha = (unsigned char *)malloc(x * y);
  if (buffer && rgb) {
    for (int p = 0; p < x * y; p++) {
      rgb[3 * p + 0] = buffer[4 * p + 0];
      rgb[3 * p + 1] = buffer[4 * p + 1];
      rgb[3 * p + 2] = buffer[4 * p + 2];
      //todo read in the below from m_settings.ppi_background_colour
      if (rgb[3 * p + 0] == 0 && rgb[3 * p + 1] == 0 && rgb[3 * p + 2] == 50){ //default background colour in the config.
        alpha[p] = 0;
      } else {
        alpha[p] = buffer[4 * p + 3]; //probably all 255's even though they shouldn't be.
      }
    }
  }
  free(buffer);
  wxImage image(x, y);
  image.SetData(rgb);
  image.SetAlpha(alpha);
  image = image.Mirror(false);
  image.SetOption("quality", 100);

  string filename = g_OciusLiveDir + '/' + name + "-capture.png";
  {
    FileLock f(filename.c_str());
    CreateFileWithPermissions(filename.c_str(), 0666);
    if (image.SaveFile(filename.c_str(), wxBITMAP_TYPE_PNG))
      OC_DEBUG("Wrote file %s.", filename.c_str());
    else
      OC_DEBUG("Failed to write file %s.", filename.c_str());
  }
  
  // Write it to a file with a comment of the timestamp
  //wxMemoryOutputStream writeBuffer;
  //if (image.SaveFile(writeBuffer, wxBITMAP_TYPE_JPEG) && writeBuffer.GetOutputStreamBuffer()->GetBufferSize() > 0) {
  //  string timestamp = MakeLocalTimeStamp();
  //  OC_TRACE("ImageSize=%d.time=%s", writeBuffer.GetOutputStreamBuffer()->GetBufferSize(), timestamp.c_str());
  //  vector<uint8_t> uncommented;
  //  uncommented.assign(reinterpret_cast<const uint8_t*>(writeBuffer.GetOutputStreamBuffer()->GetBufferStart()), reinterpret_cast<const uint8_t*>(writeBuffer.GetOutputStreamBuffer()->GetBufferEnd()));
  //  auto commented = JpegAppendComment(uncommented, timestamp, "radar0");

  //  OC_TRACE("%s=%d\n", filename.c_str(), commented.size());
  //  ofstream imageFile;
  //  CreateFileWithPermissions(filename.c_str(), 0666);
  //  imageFile.open(filename.c_str(), ios::out | ios::binary);
  //  // TODO: Do file locking
  //  imageFile.write(reinterpret_cast<const char *>(&commented.front()), commented.size());
  //  if (imageFile)
  //    OC_DEBUG("Wrote %d bytes to file %s:%s.", commented.size(), filename.c_str(), timestamp.c_str());
  //  else
  //    OC_DEBUG("Failed to write file %s.", filename.c_str());
  //  imageFile.close();
  //}
  //else {
  //  OC_DEBUG("ERROR: Faled to write GL image to buffer.");
  //}
  
  OC_TRACE("[OciusDumpVertexImage]<<");
}
