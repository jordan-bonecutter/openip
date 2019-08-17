/* * * * * * * * * * * * * * * * * * * * * * * * * * */
/* imgio.h * * * * * * * * * * * * * * * * * * * * * */
/* created by: jordan bonecutter * * * * * * * * * * */
/* * * * * * * * * * * * * * * * * * * * * * * * * * */
/* * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef __IMGIO_H__
#define __IMGIO_H__

#include "openip_globals.h"

typedef enum{
  img_ioerr_success         = 0,
  img_ioerr_file_not_found  = 1,
  img_ioerr_libjpeg_turbo   = 2,
  img_ioerr_out_of_memory   = 3,
  img_ioerr_not_a_png_file  = 4,
  img_ioerr_libpng          = 5,
  img_ioerr_unsupported_opt = 6,
  img_ioerr_could_not_open  = 7,
  img_ioerr_file_write      = 8,
  img_ioerr_unknown_ftype   = 9
}img_ioerr;

/*
char* img_ioerr_str[] = {"success", "file not found", "libjpeg-turbo internal error", 
                        "out of memory", "not a png file", "libpng internal error",
                        "unsupported feature detected", "could not open file",
                        "could not write to file", "unknown file type"};
                        */

img_ioerr img_free(img* i);

/* load image */
img* img_from_file(const char* fname, img_ioerr* err, bool fast, int qual);
img* img_from_jpeg(const char* fname, img_ioerr* err, bool fast, int qual);
img* img_from_png (const char* fname, img_ioerr* err);

/* save image */
img_ioerr img_save_to_file(img* im, const char* fname, bool fast, int qual);
img_ioerr img_save_to_jpeg(img* im, const char* fname, bool fast, int qual);
img_ioerr img_save_to_png (img* im, const char* fname);

#endif
