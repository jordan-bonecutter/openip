/* * * * * * * * * * * * * * * * * * * * * * * * * * */
/* ffimg.c * * * * * * * * * * * * * * * * * * * * * */
/* created by: jordan bonecutter * * * * * * * * * * */
/* * * * * * * * * * * * * * * * * * * * * * * * * * */
/* * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "ffimg.h"
#include <fftw3.h>
#include <stdlib.h>
#include <string.h>

#define REDOFF    0
#define GREENOFF  1
#define BLUEOFF   2
#define ALPHAOFF  3

ffimg* img_fftransform(img*   im);
fimg*  ffimg_transform(ffimg* im);
img*   img_from_fimg  (fimg*  im, fimg2img_strategy);

static void ffimcpy(gray_t* p, double* d, int tot, int offs)
{
  int i;
  for(i = 0; i < tot; i++)
  {
    d[i] = ((double)p[4*i + offs])/tot;
  }
  return;
}

ffimg* img_fftransform(img* im)
{
  fftw_plan p;
  double* t;
  fftw_complex* T;
  int i;
  ffimg* ret;

  ret = malloc(sizeof(ffimg));

  ret->red = malloc(sizeof(fcgray_t*)*im->height);
  ret->red[0] = malloc(sizeof(fcgray_t)*(im->width/2 + 1)*im->height);
  for(i = 1; i < im->height; i++)
  {
    ret->red[i] = ret->red[i-1] + (im->width/2 + 1);
  }
  ret->green = malloc(sizeof(fcgray_t*)*im->height);
  ret->green[0] = malloc(sizeof(fcgray_t)*(im->width/2 + 1)*im->height);
  for(i = 1; i < im->height; i++)
  {
    ret->green[i] = ret->green[i-1] + (im->width/2 + 1);
  }
  ret->blue = malloc(sizeof(fcgray_t*)*im->height);
  ret->blue[0] = malloc(sizeof(fcgray_t)*(im->width/2 + 1)*im->height);
  for(i = 1; i < im->height; i++)
  {
    ret->blue[i] = ret->blue[i-1] + (im->width/2 + 1);
  }
  ret->alpha = malloc(sizeof(fcgray_t*)*im->height);

  t = fftw_malloc(sizeof(double)*im->width*im->height);
  ffimcpy((gray_t*)(*im->pix), t, im->width*im->height, REDOFF);
  T = fftw_malloc(sizeof(fftw_complex)*(im->width/2 + 1)*im->height);
  p = fftw_plan_dft_r2c_2d(im->height, im->width, t, T, 0);
  fftw_execute(p);
  memcpy((*ret->red), T, sizeof(fftw_complex)*(im->width/2 + 1)*im->height);

  ffimcpy((gray_t*)(*im->pix), t, im->width*im->height, GREENOFF);
  fftw_execute(p);
  memcpy((*ret->green), T, sizeof(fftw_complex)*(im->width/2 + 1)*im->height);

  ffimcpy((gray_t*)(*im->pix), t, im->width*im->height, BLUEOFF);
  fftw_execute(p);
  memcpy((*ret->blue), T, sizeof(fftw_complex)*(im->width/2 + 1)*im->height);

  ffimcpy((gray_t*)(*im->pix), t, im->width*im->height, ALPHAOFF);
  fftw_execute(p);
  ret->alpha[0] = (fcgray_t*)T;
  for(i = 1; i < im->height; i++)
  {
    ret->alpha[i] = ret->alpha[i-1] + (im->width/2 + 1);
  }
  fftw_destroy_plan(p);
  fftw_free(t);

  return ret;
}
