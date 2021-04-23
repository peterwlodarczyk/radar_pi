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

static system_clock::time_point next_update = system_clock::now();
static int oc_count = 0;
int height = 0;
int width = 0; 
int inputSize = width*height * 4;
//setup buffers to reuse
unsigned char *buffer = nullptr;
png_bytep * row_pointers = nullptr;


void malloc_row_buffers(){ //called on first image save and if width/height changes
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

void abort_(const char * s, ...)
{
	va_list args;
	va_start(args, s);
	vfprintf(stderr, s, args);
	fprintf(stderr, "\n");
	va_end(args);
  abort();
}


void write_png_file(char* file_name, png_infop info_ptr, png_bytep * row_pointers)
{
  if (buffer == nullptr || row_pointers == nullptr) 
  {
    malloc_row_buffers(); //first time we have run this save function allocate what we will use.
  }
 
	/* create file */
	FILE *fp = fopen(file_name, "wb");
	if (!fp)
		abort_("[write_png_file] File %s could not be opened for writing", file_name);

  png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

	if (!png_ptr)
		abort_("[write_png_file] png_create_write_struct failed");

	if (!info_ptr) //should already exist - given to us in function call.
		abort_("[read_png_file] png_create_info_struct failed");

	if (setjmp(png_jmpbuf(png_ptr)))
		abort_("[write_png_file] Error during init_io");

	png_init_io(png_ptr, fp);
  
	/* write header */
	if (setjmp(png_jmpbuf(png_ptr)))
		abort_("[write_png_file] Error during writing header");

  int color_type = png_get_color_type(png_ptr, info_ptr);
	int bit_depth = png_get_bit_depth(png_ptr, info_ptr);

#if 0
	png_set_IHDR(png_ptr, info_ptr, width, height,
			bit_depth, color_type, PNG_INTERLACE_NONE,
			PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
#endif
	png_write_info(png_ptr, info_ptr);
	/* write bytes */
	if (setjmp(png_jmpbuf(png_ptr)))
		abort_("[write_png_file] Error during writing bytes");

	png_write_image(png_ptr, row_pointers);
	/* end write */
	if (setjmp(png_jmpbuf(png_ptr)))
		abort_("[write_png_file] Error during end of write");

	png_write_end(png_ptr, NULL);
  png_destroy_write_struct(&png_ptr, &info_ptr); //also destorys the info_ptr. 
	fclose(fp);
}

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
  if (width != viewport[2] || height != viewport[3])
  {
    width = viewport[2];
    height = viewport[3];
    inputSize = width*height * 4;
    //width/height changed so remake the structs.
    malloc_row_buffers(); 
  }
 
  glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
  //set the transparent sections based on the 0,0,50 (initial background settings)
  if (buffer){
    for (int p = 0; p < width * height * 4; p += 4){ //for each RGBA section
      if (buffer[p + 0] == 0 && buffer[p + 1] == 0 && buffer[p + 2] == 50){ //default background colour in the config.
      //todo check if there is a better way to do the above check (compare bits of the whole section?)
        buffer[p + 3] = 0;
      }
    }
  }

  //use opting to evaluate image compression. 
  // optipng -full image.png specifies the best output format for the image.
  //todo reduce the total colours used in the radar to < 8 to reduce file size again.
  for (int i = 0; i < height; i ++){
    memcpy(row_pointers[i], buffer + ((height*width*4) - ((i+1)*width*4)), width*4); //mirroring the image as we allocate
  }
  if(1)
  {
    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr)
      abort_("[read_png_file] png_create_read_struct failed");
    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
      abort_("[read_png_file] png_create_info_struct failed");
    // //png_bytep is a typedef unsigned char png_byte5  
    png_set_IHDR(png_ptr, info_ptr, width, height, 8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE , PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_set_rows(png_ptr, info_ptr, row_pointers);
    png_uint_32 reductions = OPNG_REDUCE_RGB_TO_PALETTE | OPNG_REDUCE_8_TO_4_2_1; //reduce the image to palette -> major compression. ~50% reduced file size.
    png_uint_32 result = opng_reduce_image(png_ptr, info_ptr, reductions);

    string filename = g_OciusLiveDir + '/' + name + "-capture.png";
    {
      FileLock f(filename.c_str());
      if (f.locked())
      {
        CreateFileWithPermissions(filename.c_str(), 0666);
        write_png_file((char*) filename.c_str(), info_ptr, row_pointers);
      }
    }
    
    png_destroy_read_struct(&png_ptr, nullptr, nullptr); //note info_ptr already destroyed
    }
  OC_TRACE("[OciusDumpVertexImage]<<");
}
