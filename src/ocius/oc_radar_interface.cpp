#include <time.h>

#include <chrono>
#include <fstream>
#include <string>

#include "oc_utils.h"
#include "pi_common.h"

using namespace std;

#ifdef WIN32
static const std::string ImageDir = "c:\\temp";
#else
//static const std::string ImageDir = "/dev/shm/usv/live";
static const std::string ImageDir = "/tmp";
#endif

static time_t next_update = 0;
static int oc_count = 0;


void OciusDumpVertexImage(int radar) {
  if (time(0) < next_update) 
    return;
  string name = string("radar") + to_string(radar);
  OC_DEBUG("[OciusDumpVertexImage] %s:%d>>", name.c_str(), oc_count);
  next_update = time(0) + 1;

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
  image.SetOption("quality", 50);

#ifdef WIN32
  string tmpFilename = string("c:\\temp\\") + name + "-tmp.jpg";
#else
  string tmpFilename = string("/tmp/") + name + "-tmp.jpg";  // ImageDir + '/' + name + "-tmp.jpg";
#endif
  if (!image.SaveFile("/tmp/radar0.bmp", wxBITMAP_TYPE_JPEG)) {
    OC_DEBUG("ERROR: Faled to write file %s", "/tmp/radar0.bmp");
  }
  if (!image.Mirror(false).SaveFile(tmpFilename.c_str(), wxBITMAP_TYPE_JPEG)) {
    OC_DEBUG("ERROR: Faled to write file %s", tmpFilename.c_str());
  }

  auto uncommented = readfile(tmpFilename.c_str());
  OC_TRACE("%s=%d\n", tmpFilename.c_str(), uncommented.size());
  if (uncommented.size()) 
  {
    OC_TRACE("time=%s\n", MakeLocalTimeStamp().c_str());
    string timestamp = MakeLocalTimeStamp();
    auto commented = JpegAppendComment(uncommented, MakeLocalTimeStamp(), "radar0");

    string filename = ImageDir + '/' + name + "-capture.jpg";
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

  OC_TRACE("[OciusDumpVertexImage]<<");
}
