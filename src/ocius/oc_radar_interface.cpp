#include <time.h>

#include <chrono>
#include <fstream>
#include <string>
#include "oc_utils.h"
#include "oc_lock.h"
#include "pi_common.h"
#include "opngreduc.h"

using namespace std;
using namespace std::chrono;

extern std::string g_OciusLiveDir;

// TODO: We should probably use seperate values for each radar here
static int oc_count = 0;
static int height = 0;
static int width = 0; 
static int inputSize = width*height * 4;
//setup buffers to reuse
static unsigned char *buffer = nullptr;
static png_bytep * row_pointers = nullptr;

static void malloc_row_buffers(){ //called on first image save and if width/height changes
  OC_DEBUG("[malloc_row_buffers]");
  //free everything before mallocing again.
  if (row_pointers != nullptr)
  {
    for (int i = 0; i < height; i ++){ //is is possible this isn't free'ing everything. ?
      if (row_pointers[i]){
        free(row_pointers[i]); //free rows_pointer contents. this will be realloced later (to allow for new rows. )
      }
    }
    free(row_pointers);
  }
  free(buffer);
  row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * height);
  buffer = (unsigned char *)malloc(inputSize);
  memset(buffer, 0, inputSize); //puts all to 0's.

  for (int i = 0; i < height ; i ++){
    row_pointers[i] = (png_bytep) malloc(width * 4);
  }
}

static bool write_png_file(const char* file_name, png_infop info_ptr, png_bytep * row_pointers)
{
  bool ret = false;
  if (buffer == nullptr || row_pointers == nullptr) 
  {
    //first time we have run this save function allocate what we will use.
    malloc_row_buffers(); 
  }

  // create file we're going to write to.
  FILE *fp = fopen(file_name, "wb");
  if (!fp)
  {
    OC_DEBUG("[write_png_file] File %s could not be opened for writing", file_name);
    return false;
  }

  png_structp png_write_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png_write_ptr)
  {
    OC_DEBUG("[write_png_file] png_create_write_struct failed");
    return false;
  }

  if (!info_ptr) //should already exist - given to us in function call.
  {
    OC_DEBUG("[read_png_file] png_create_info_struct failed");
    return false;
  }

  png_init_io(png_write_ptr, fp);
  if (setjmp(png_jmpbuf(png_write_ptr)))
  {
    OC_DEBUG("[write_png_file] Error during init_io");
    return false;
  }

  int color_type = png_get_color_type(png_write_ptr, info_ptr);
  int bit_depth = png_get_bit_depth(png_write_ptr, info_ptr);

  //png_set_IHDR(png_write_ptr, info_ptr, width, height, bit_depth, color_type, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

  png_write_info(png_write_ptr, info_ptr);
  if (setjmp(png_jmpbuf(png_write_ptr)))
  {
    OC_DEBUG("[write_png_file] Error during writing info");
    return false;
  }

  png_write_image(png_write_ptr, row_pointers);
  if (setjmp(png_jmpbuf(png_write_ptr)))
  {
    OC_DEBUG("[write_png_file] Error writing image");
    return false;
  }

  png_write_end(png_write_ptr, NULL);
  if (setjmp(png_jmpbuf(png_write_ptr)))
  {
    OC_DEBUG("[write_png_file] Error during end of write");
    return false;
  }

  png_destroy_write_struct(&png_write_ptr, &info_ptr); //also destorys the info_ptr. 
  fclose(fp);
  ret = true;
  return true;
}

bool OciusDumpVertexImage(int radar, string stage) {
  if (radar < 0 || radar > 1)
    return false;
  ProfilerGuardT tg(OCIUSDUMPVERTEXIMAGE);
  bool ret = false;
  string name;
  if (radar == 1)
    name = string("radarb");
  else
    name = string("radara");

  OC_TRACE("[%s] %s:%d>>", __func__, name.c_str(), oc_count);
  ++oc_count;

  GLint viewport[4];
  glGetIntegerv(GL_VIEWPORT, viewport);
  if (width != viewport[2] || height != viewport[3]) {
    width = viewport[2];
    height = viewport[3];
    inputSize = width*height * 4;
    //width/height changed so remake the structs.
    malloc_row_buffers(); 
  }

  glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
  //set the transparent sections based on the 0,0,50 (initial background settings)
  if (buffer) {
    for (int p = 0; p < width * height * 4; p += 4) { //for each RGBA section
      if (buffer[p + 0] == 0 && buffer[p + 1] == 0 && buffer[p + 2] == 50) { //default background colour in the config.
      //todo check if there is a better way to do the above check (compare bits of the whole section?)
        if(stage == "overlay")
          buffer[p + 3] = 0;
        else
          buffer[p + 2] = 0; //black bg for non-overlay
      } else if (buffer[p + 0] == 0 && buffer[p + 1] == 0 && buffer[p + 2] == 94) { //GZ B colour
        buffer[p + 3] = 30;
      } else if (buffer[p + 0] == 0 && buffer[p + 1] == 55 && buffer[p + 2] == 39) { // GZ A colour
        buffer[p + 3] = 30;
      } else if (buffer[p + 0] == 0 && buffer[p + 1] == 43 && buffer[p + 2] == 86) { // GZ overlapping colour
        buffer[p + 3] = 30;
      }
    }
  }

  //use opting to evaluate image compression. 
  // optipng -full image.png specifies the best output format for the image.
  //todo reduce the total colours used in the radar to < 8 to reduce file size again.
  for (int i = 0; i < height; i ++) {
    memcpy(row_pointers[i], buffer + ((height*width*4) - ((i+1)*width*4)), width*4); //mirroring the image as we allocate
  }

  png_structp png_read_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png_read_ptr)
  {
    OC_TRACE("[read_png_file] png_create_read_struct failed");
    return false;
  }
  png_infop info_ptr = png_create_info_struct(png_read_ptr);
  if (!info_ptr)
  {
    OC_TRACE("[read_png_file] png_create_info_struct failed");
    return false;
  }
  // //png_bytep is a typedef unsigned char png_byte5  
  png_set_IHDR(png_read_ptr, info_ptr, width, height, 8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE , PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
  png_set_rows(png_read_ptr, info_ptr, row_pointers);
  png_uint_32 reductions = OPNG_REDUCE_RGB_TO_PALETTE | OPNG_REDUCE_8_TO_4_2_1; //reduce the image to palette -> major compression. ~50% reduced file size.
  png_uint_32 reduce_result = opng_reduce_image(png_read_ptr, info_ptr, reductions);
  OC_TRACE("[OciusDumpVertexImage] opng_reduce_image=%d.", reduce_result);

  string filename = g_OciusLiveDir + '/' + name + '-' + stage + ".png"; 
  {
    FileLock f(filename.c_str());
    if (f.locked()) {
      // The file must be readable by the camera capture process
      (void)CreateFileWithPermissions(filename.c_str(), 0666);
      ret = write_png_file(filename.c_str(), info_ptr, row_pointers);
      if (!ret)
        OC_TRACE("[OciusDumpVertexImage] Failed to write png file %s.", filename.c_str());
    }
  }
  png_destroy_read_struct(&png_read_ptr, nullptr, nullptr); //note info_ptr already destroyed
  return ret;
}
