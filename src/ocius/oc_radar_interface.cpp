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
	/* create file */
	FILE *fp = fopen(file_name, "wb");
	if (!fp)
		abort_("[write_png_file] File %s could not be opened for writing", file_name);


	/* initialize stuff */
	png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

	if (!png_ptr)
		abort_("[write_png_file] png_create_write_struct failed");

	//info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
		abort_("[write_png_file] png_create_info_struct failed");

	if (setjmp(png_jmpbuf(png_ptr)))
		abort_("[write_png_file] Error during init_io");

	png_init_io(png_ptr, fp);


	/* write header */
	if (setjmp(png_jmpbuf(png_ptr)))
		abort_("[write_png_file] Error during writing header");

	int width = png_get_image_width(png_ptr, info_ptr);
	int height = png_get_image_height(png_ptr, info_ptr);
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

	/* cleanup heap allocation */
  /*
	for (inty=0; y<height; y++)
		free(row_pointers[y]);
	free(row_pointers);
  */
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

  int width = viewport[2];
  int height = viewport[3];
  int inputSize = width*height * 4;
  unsigned char *buffer = (unsigned char *)malloc(inputSize);
  memset(buffer, 0, inputSize);
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

  /*
  int inputSum = 0;
  for (int i = 0; i < inputSize; ++i)
    inputSum += buffer[i];
  OC_TRACE("Vierport:%d,%d,%d,%d.inputSum=%d.\n", viewport[0], viewport[1], viewport[2], viewport[3], inputSum);
  */
  
  //todo replace the wxImage save procedure here with using libpng directly.
  //todo convert the RGBa buffer created to a 4bitdepth pallete (9 colours - 8 + transparency) when using libpng to save.
  //use opting to evaluate image compression. 
  // optipng -full image.png specifies the best output format for the image.
  //todo reduce the total colours used in the radar to < 8 to reduce file size again.
  png_bytep * row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * height);
  for (int i = 0; i < height; i ++){
    row_pointers[i] = (png_bytep) malloc(width * 4);
    memcpy(row_pointers[i], buffer + ((height*width*4) - ((i+1)*width*4)), width*4); //mirroring the image as we allocate
  }

  png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr)
		abort_("[read_png_file] png_create_read_struct failed");
	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
		abort_("[read_png_file] png_create_info_struct failed");

  //IHDR -> our current settings

  // //png_bytep is a typedef unsigned char png_byte5  
  png_set_IHDR(png_ptr, info_ptr, width, height, 8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE , PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
  png_set_rows(png_ptr, info_ptr, row_pointers);
  png_uint_32 reductions = OPNG_REDUCE_RGB_TO_PALETTE | OPNG_REDUCE_8_TO_4_2_1; //reduce the image to palette -> major compression. ~50% reduced file size.
	png_uint_32 result = opng_reduce_image(png_ptr, info_ptr, reductions);

  /*
  int outputSize = x * y * 3;
  unsigned char *rgb = (unsigned char *)malloc(outputSize); //this wasn't being free'd... memory leaks?
  unsigned char *alpha = (unsigned char *)malloc(x * y); //this wasn't being free'd... memory leaks?
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
  */
  //turn buffer into png_ptr & info_ptr -> this might be too expensive, possibly make these global and just modify them over and over?
  /*
  wxImage image(x, y);
  image.SetData(rgb);
  image.SetAlpha(alpha);
  image = image.Mirror(false);  
  image.SetOption(wxIMAGE_OPTION_PNG_BITDEPTH, 8); //minimum bit depth. //todo change to palette if possible.
  //the below options are determined by optipng
  image.SetOption(wxIMAGE_OPTION_PNG_COMPRESSION_LEVEL, 9);
  image.SetOption(wxIMAGE_OPTION_PNG_COMPRESSION_MEM_LEVEL, 8); 
  image.ConvertAlphaToMask(wxIMAGE_ALPHA_THRESHOLD); 
  image.SetOption(wxIMAGE_OPTION_PNG_COMPRESSION_STRATEGY, 1);
  */
  string filename = g_OciusLiveDir + '/' + name + "-capture.png";
  {
    FileLock f(filename.c_str());
    if (f.locked())
    {
      CreateFileWithPermissions(filename.c_str(), 0666);
      write_png_file((char*) filename.c_str(), info_ptr, row_pointers);
      /*
      if (image.SaveFile(filename.c_str(), wxBITMAP_TYPE_PNG))
        OC_DEBUG("Wrote file %s.", filename.c_str());
      else
        OC_DEBUG("Failed to write file %s.", filename.c_str());
      */
    }
  }

  free(buffer);
  for (int i = 0; i < height; i ++){
    if (row_pointers[i]){
      free(row_pointers[i]);
    }
  }
  free(row_pointers);
  OC_TRACE("[OciusDumpVertexImage]<<");
}
