#include <time.h>

#include <chrono>
#include <fstream>
#include <string>

#include "oc_utils.h"
#include "pi_common.h"

using namespace std;

static const std::string ImageDir = "/dev/shm/usv/live";
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
  unsigned char *buffer = (unsigned char *)malloc(x * y * 4);
  glReadPixels(0, 0, x, y, GL_RGBA, GL_UNSIGNED_BYTE, buffer);

  unsigned char *e = (unsigned char *)malloc(x * y * 3);
  if (buffer && e) {
    for (int p = 0; p < x * y; p++) {
      e[3 * p + 0] = buffer[4 * p + 0];
      e[3 * p + 1] = buffer[4 * p + 1];
      e[3 * p + 2] = buffer[4 * p + 2];
    }
  }
  free(buffer);

  wxImage image0(x, y);
  image0.SetData(e);
  wxImage image1 = image0.Mirror(true);
  wxImage image2 = image0.Mirror(false);

  OC_TRACE("Vierport:%d,%d,%d,%d\n", viewport[0], viewport[1], viewport[2], viewport[3]);
  image2.SetOption("quality", 50);

  string tmpFilename = ImageDir + '/' + name + "-tmp.jpg";
  image2.SaveFile(tmpFilename.c_str(), wxBITMAP_TYPE_JPEG);

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
