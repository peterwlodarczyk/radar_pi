#include <time.h>

#include <chrono>
#include <fstream>
#include <string>

#include "oc_utils.h"
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
  string name = string("radar") + to_string(radar);
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
  unsigned char *e = (unsigned char *)malloc(outputSize);
  if (buffer && e) {
    for (int p = 0; p < x * y; p++) {
      e[3 * p + 0] = buffer[4 * p + 0];
      e[3 * p + 1] = buffer[4 * p + 1];
      e[3 * p + 2] = buffer[4 * p + 2];
    }
  }
  free(buffer);
  wxImage image(x, y);
  image.SetData(e);
  image = image.Mirror(false);
  image.SetOption("quality", 90);

  // Write it to a file with a comment of the timestamp
  wxMemoryOutputStream writeBuffer;
  if (image.SaveFile(writeBuffer, wxBITMAP_TYPE_JPEG) && writeBuffer.GetOutputStreamBuffer()->GetBufferSize() > 0) {
    string timestamp = MakeLocalTimeStamp();
    OC_TRACE("ImageSize=%d.time=%s", writeBuffer.GetOutputStreamBuffer()->GetBufferSize(), timestamp.c_str());
    vector<uint8_t> uncommented;
    uncommented.assign(reinterpret_cast<const uint8_t*>(writeBuffer.GetOutputStreamBuffer()->GetBufferStart()), reinterpret_cast<const uint8_t*>(writeBuffer.GetOutputStreamBuffer()->GetBufferEnd()));
    auto commented = JpegAppendComment(uncommented, timestamp, "radar0");

    string filename = g_OciusLiveDir + '/' + name + "-capture.jpg";
    OC_TRACE("%s=%d\n", filename.c_str(), commented.size());
    ofstream commentedFile;
    CreateFileWithPermissions(filename.c_str(), 0666);
    commentedFile.open(filename.c_str(), ios::out | ios::binary);
    // TODO: Do file locking
    commentedFile.write(reinterpret_cast<const char *>(&commented.front()), commented.size());
    if (commentedFile)
      OC_DEBUG("Wrote %d bytes to file %s:%s.", commented.size(), filename.c_str(), timestamp.c_str());
    else if (commentedFile)
      OC_DEBUG("Failed to write file %s.", filename.c_str());
    commentedFile.close();
  }
  else {
    OC_DEBUG("ERROR: Faled to write GL image to buffer.");
  }
  
  OC_TRACE("[OciusDumpVertexImage]<<");
}
