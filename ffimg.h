/* * * * * * * * * * * * * * * * * * * * * * * * * * */
/* ffimg.h * * * * * * * * * * * * * * * * * * * * * */
/* created by: jordan bonecutter * * * * * * * * * * */
/* * * * * * * * * * * * * * * * * * * * * * * * * * */
/* * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef __FFIMG_H__
#define __FFIMG_H__

#include "openip_globals.h"

typedef enum{
  fimg2img_direct,
  fimg2img_scaled
}fimg2img_strategy;

typedef double       fgray_t;
typedef struct{fgray_t re, im;}fcgray_t;
typedef struct{fgray_t**  red, **blue, **green, **alpha; int width, height;}fimg;
typedef struct{fcgray_t** red, **blue, **green, **alpha; int width, height;}ffimg;

ffimg* img_fftransform(img*   im);
fimg*  ffimg_transform(ffimg* im);
img*   img_from_fimg  (fimg*  im, fimg2img_strategy);
void   ffimg_free     (ffimg* im);
void   fimg_free      (fimg*  im);

#endif
