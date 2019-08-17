/* * * * * * * * * * * * * * * * * * * * * * * * * * */
/* imgio.c * * * * * * * * * * * * * * * * * * * * * */
/* created by: jordan bonecutter * * * * * * * * * * */
/* * * * * * * * * * * * * * * * * * * * * * * * * * */
/* * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "imgio.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <turbojpeg.h>
#include <png.h>
#include <regex.h>

#define PNG_CMP_NUMBER  8

static pixel** rgba_convert_to_pixels      (uint8_t** pix, int width, int height, int bitdepth);
static pixel** rgb_convert_to_pixels       (uint8_t** pix, int width, int height, int bitdepth);
static pixel** gray_alpha_convert_to_pixels(uint8_t** pix, int width, int height, int bitdepth);
static pixel** gray_convert_to_pixels      (uint8_t** pix, int width, int height, int bitdepth);

img_ioerr img_free(img* i)
{
  free(*i->pix);
  free(i->pix);
  free(i);
  return img_ioerr_success;
}

img* img_from_file(const char* fname, img_ioerr* err, bool fast, int qual)
{
  regex_t png_regex;
  regex_t jpg_regex;
  int match;

  regcomp(&png_regex, "\\.png$", 0);
  match = regexec(&png_regex, fname, 0, NULL, 0);
  regfree(&png_regex);
  if(!match)
  {
    *err = img_ioerr_success;
    return img_from_png(fname, err);
  }

  regcomp(&jpg_regex, "\\.jpg$", 0);
  match = regexec(&jpg_regex, fname, 0, NULL, 0);
  regfree(&jpg_regex);
  if(!match)
  {
    *err = img_ioerr_success;
    return img_from_jpeg(fname, err, fast, qual);
  }

  *err = img_ioerr_unknown_ftype;
  return NULL;
}

img* img_from_jpeg(const char* fname, img_ioerr* err, bool fast, int qual)
{
  /* local var */
  img* ret;
  tjhandle tjinstance = NULL;
  unsigned char* jpgBuf;
  FILE *jpgFile;
  int flen, insubsamp, incspace, flags, i;
  
  if((jpgFile = fopen(fname, "rb")) == NULL) 
  {
    *err = img_ioerr_file_not_found;
    return NULL; 
  }

  fseek(jpgFile, 0, SEEK_END);
  flen = ftell(jpgFile);
  fseek(jpgFile, 0, SEEK_SET);

  jpgBuf = tjAlloc(flen*sizeof(char));
  fread(jpgBuf, sizeof(char), flen, jpgFile);
  fclose(jpgFile);

  if((tjinstance = tjInitDecompress()) == NULL)
  {
    free(jpgBuf);
    *err = img_ioerr_libjpeg_turbo;
    return NULL;
  }

  ret = malloc(sizeof(img));
  if(!ret)
  {
    free(jpgBuf);
    tjDestroy(tjinstance);
    *err = img_ioerr_out_of_memory;
    return NULL;
  }

  if(tjDecompressHeader3(tjinstance, (unsigned char*)jpgBuf, flen, &ret->width, &ret->height, &insubsamp, &incspace) < 0)
  {
    free(jpgBuf);
    free(ret);
    tjDestroy(tjinstance);
    *err = img_ioerr_libjpeg_turbo;
    return NULL;
  }

  if((ret->pix = malloc(ret->height*sizeof(pixel*))) == NULL)
  {
    free(jpgBuf);
    free(ret);
    tjDestroy(tjinstance);
    *err = img_ioerr_out_of_memory;
    return NULL;
  }
  if((ret->pix[0] = malloc(ret->width*ret->height*sizeof(pixel))) == NULL)
  {
    free(jpgBuf);
    free(ret);
    free(ret->pix);
    tjDestroy(tjinstance);
    *err = img_ioerr_out_of_memory;
    return NULL;
  }
  for(i = 1; i < ret->height; i++)
  {
    ret->pix[i] = ret->pix[i-1] + ret->width;
  }

  flags = 0;
  if(fast)
  {
    flags |= TJFLAG_FASTDCT; 
  }
  else
  {
    flags |= TJFLAG_ACCURATEDCT; 
  }
  if(tjDecompress2(tjinstance, (unsigned char*)jpgBuf, flen, (unsigned char*)*ret->pix, ret->width, 0, ret->height, TJPF_RGBA, flags) < 0)
  {
    tjFree(jpgBuf);
    tjDestroy(tjinstance);
    *err = img_ioerr_libjpeg_turbo;
    return NULL;
  }

  tjDestroy(tjinstance);
  free(jpgBuf);

  *err = img_ioerr_success;
  return ret;
}

img* img_from_png(const char* fname, img_ioerr* err)
{
  /* local var */
  img* ret;
  uint8_t** tmp;
  char header[8];
  FILE* pngFile;
  png_structp png_ptr;
  png_infop info_ptr;
  int rowbytes, bitdepth, colortype, i, channels;

  if((pngFile = fopen(fname, "rb")) == NULL)
  {
    *err = img_ioerr_file_not_found;
    return NULL;
  }

  if(fread(header, sizeof(char), PNG_CMP_NUMBER, pngFile) != PNG_CMP_NUMBER)
  {
    *err = img_ioerr_not_a_png_file;
    fclose(pngFile);
    return NULL;
  }

  if(png_sig_cmp((png_const_bytep)header, 0, PNG_CMP_NUMBER) != 0)
  {
    *err = img_ioerr_not_a_png_file;
    fclose(pngFile);
    return NULL;
  }

  if((png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL)) == NULL)
  {
    *err = img_ioerr_libpng;
    fclose(pngFile);
    return NULL;
  }

  if((info_ptr = png_create_info_struct(png_ptr)) == NULL)
  {
    *err = img_ioerr_libpng;
    png_destroy_read_struct(&png_ptr, NULL, NULL);
    fclose(pngFile);
    return NULL;
  }

  png_init_io(png_ptr, pngFile);
  png_set_sig_bytes(png_ptr, PNG_CMP_NUMBER);
  png_read_info(png_ptr, info_ptr);
  
  if((ret = malloc(sizeof(img))) == NULL)
  {
    *err = img_ioerr_out_of_memory;
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    fclose(pngFile);
    return NULL;
  }
  ret->width  = png_get_image_width (png_ptr, info_ptr);
  ret->height = png_get_image_height(png_ptr, info_ptr);
  if((colortype   = png_get_color_type  (png_ptr, info_ptr)) == PNG_COLOR_TYPE_PALETTE)
  {
    *err = img_ioerr_unsupported_opt;
    free(ret);
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    fclose(pngFile);
    return NULL;
  }
  if((bitdepth    = png_get_bit_depth   (png_ptr, info_ptr)) < 8)
  {
    *err = img_ioerr_unsupported_opt;
    free(ret);
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    fclose(pngFile);
    return NULL;
  }
  rowbytes    = png_get_rowbytes    (png_ptr, info_ptr);
  channels    = png_get_channels    (png_ptr, info_ptr);

  if((tmp = malloc(sizeof(uint8_t*)*ret->height)) == NULL)
  {
    *err = img_ioerr_out_of_memory;
    free(ret);
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    fclose(pngFile);
    return NULL;
  }
  if((*tmp = malloc(rowbytes*ret->height)) == NULL)
  {
    *err = img_ioerr_out_of_memory;
    free(ret);
    free(tmp);
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    fclose(pngFile);
    return NULL;
  }
  for(i = 1; i < ret->height; i++)
  {
    tmp[i] = tmp[i-1] + rowbytes/sizeof(uint8_t);
  }

  png_read_image(png_ptr, tmp);

  png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
  fclose(pngFile);

  switch(channels)
  {
    // gray
    case 1:
      ret->pix = gray_convert_to_pixels(tmp, ret->width, ret->height, bitdepth);
      break;

    // gray_alpha
    case 2:
      ret->pix = gray_alpha_convert_to_pixels(tmp, ret->width, ret->height, bitdepth);
      break;

    // rgb
    case 3:
      ret->pix = rgb_convert_to_pixels(tmp, ret->width, ret->height, bitdepth);
      break;

    // rgba
    case 4:
      ret->pix = rgba_convert_to_pixels(tmp, ret->width, ret->height, bitdepth);
      break;

    // ?
    default:
      *err = img_ioerr_unsupported_opt;
      free(*tmp);
      free(tmp);
      return NULL;
  }

  return ret; 
}

static pixel** rgba_convert_to_pixels(uint8_t** pix, int width, int height, int bitdepth)
{
  pixel** ret;
  int i;
  if(bitdepth == sizeof(gray_t))
  {
    return (pixel**)pix; 
  }

  if((ret = malloc(sizeof(pixel*)*height)) == NULL)
  {
    return NULL; 
  }

  *ret = malloc(sizeof(pixel)*width*height);
  for(i = 1; i < height; i++)
  {
    ret[i] = ret[i-1] + width;
  }

  for(i = 0; i < width*height; i++)
  {
    (*ret)[i].r = (*pix)[4*i]    >>(bitdepth-sizeof(gray_t)*8);
    (*ret)[i].g = (*pix)[4*i + 1]>>(bitdepth-sizeof(gray_t)*8);
    (*ret)[i].b = (*pix)[4*i + 2]>>(bitdepth-sizeof(gray_t)*8);
    (*ret)[i].a = (*pix)[4*i + 3]>>(bitdepth-sizeof(gray_t)*8);
  }

  free(*pix);
  free(pix);
  return ret;
}

static pixel** rgb_convert_to_pixels(uint8_t** pix, int width, int height, int bitdepth)
{
  pixel** ret;
  int i;

  if((ret = malloc(sizeof(pixel*)*height)) == NULL)
  {
    return NULL; 
  }

  *ret = malloc(sizeof(pixel)*width*height);
  for(i = 1; i < height; i++)
  {
    ret[i] = ret[i-1] + width;
  }

  for(i = 0; i < width*height; i++)
  {
    (*ret)[i].r = (*pix)[3*i]    >>(bitdepth-8*sizeof(gray_t));
    (*ret)[i].g = (*pix)[3*i + 1]>>(bitdepth-8*sizeof(gray_t));
    (*ret)[i].b = (*pix)[3*i + 2]>>(bitdepth-8*sizeof(gray_t));
    (*ret)[i].a = 255;
  }

  free(*pix);
  free(pix);
  return ret;
}

static pixel** gray_alpha_convert_to_pixels(uint8_t** pix, int width, int height, int bitdepth)
{
  pixel** ret;
  int i, val;
  if((ret = malloc(sizeof(pixel*)*height)) == NULL)
  {
    return NULL; 
  }

  *ret = malloc(sizeof(pixel)*width*height);
  for(i = 1; i < height; i++)
  {
    ret[i] = ret[i-1] + width;
  }

  for(i = 0; i < width*height; i++)
  {
    val = (*pix)[2*i]>>(bitdepth-8*sizeof(gray_t));
    (*ret)[i].r = val;
    (*ret)[i].g = val;
    (*ret)[i].b = val;
    (*ret)[i].a = 255;
  }

  free(*pix);
  free(pix);
  return ret;
}

static pixel** gray_convert_to_pixels(uint8_t** pix, int width, int height, int bitdepth)
{
  pixel** ret;
  int i, val;
  if((ret = malloc(sizeof(pixel*)*height)) == NULL)
  {
    return NULL; 
  }

  *ret = malloc(sizeof(pixel)*width*height);
  for(i = 1; i < height; i++)
  {
    ret[i] = ret[i-1] + width;
  }

  for(i = 0; i < width*height; i++)
  {
    val = (*pix)[i]>>(bitdepth-8*sizeof(gray_t));
    (*ret)[i].r = val;
    (*ret)[i].g = val;
    (*ret)[i].b = val;
    (*ret)[i].a = 255;
  }

  free(*pix);
  free(pix);
  return ret;
}

img_ioerr img_save_to_file(img* im, const char* fname, bool fast, int qual)
{
  regex_t png_regex;
  regex_t jpg_regex;
  int match;

  regcomp(&png_regex, "\\.png$", 0);
  match = regexec(&png_regex, fname, 0, NULL, 0);
  regfree(&png_regex);
  if(!match)
  {
    img_save_to_png(im, fname);
    return img_ioerr_success;
  }

  regcomp(&jpg_regex, "\\.jpg$", 0);
  match = regexec(&jpg_regex, fname, 0, NULL, 0);
  regfree(&jpg_regex);
  if(!match)
  {
    img_save_to_jpeg(im, fname, fast, qual);
    return img_ioerr_success;
  }

  return img_ioerr_unknown_ftype;
}

img_ioerr img_save_to_jpeg(img* im, const char* fname, bool fast, int qual)
{
  unsigned long jpgSize;
  unsigned char* jpgBuf;
  tjhandle tjinstance;
  int flags;
  FILE* jpgFile;

  if((tjinstance = tjInitCompress()) == NULL)
  {
    return img_ioerr_libjpeg_turbo;
  }

  qual = qual < 1 ? 1 : (qual > 100 ? 100 : qual);
  flags = 0;
  if(fast)
  {
    flags |= TJFLAG_FASTDCT; 
  }
  else
  {
    flags |= TJFLAG_ACCURATEDCT; 
  }
  jpgBuf = NULL;
  if((tjCompress2(tjinstance, (unsigned char*)*im->pix, im->width, 0, im->height, TJPF_RGBA, &jpgBuf, &jpgSize, TJSAMP_420, qual, flags)) < 0)
  {
    tjDestroy(tjinstance);
    return img_ioerr_libjpeg_turbo;
  }
  tjDestroy(tjinstance);
  
  if((jpgFile = fopen(fname, "wb")) == NULL)
  {
    tjFree(jpgBuf);
    return img_ioerr_could_not_open;
  }

  if((fwrite(jpgBuf, jpgSize, sizeof(char), jpgFile)) < 1)
  {
    tjFree(jpgBuf);
    fclose(jpgFile);
    return img_ioerr_file_write;
  }

  tjFree(jpgBuf);
  fclose(jpgFile);
  return 0;
}

img_ioerr img_save_to_png(img* im, const char* fname)
{
  FILE* pngFile;
  png_structp png_ptr;
  png_infop info_ptr;

  if((pngFile = fopen(fname, "wb")) == NULL)
  {
    return img_ioerr_could_not_open; 
  }

  if((png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL)) == NULL)
  {
    fclose(pngFile);
    return img_ioerr_libpng;
  }

  if((info_ptr = png_create_info_struct(png_ptr)) == NULL)
  {
    fclose(pngFile);
    png_destroy_write_struct(&png_ptr, NULL);
    return img_ioerr_libpng;
  }

  png_init_io(png_ptr, pngFile);
  png_set_IHDR(png_ptr, info_ptr, im->width, im->height, 8, 
              PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
              PNG_FILTER_TYPE_DEFAULT);

  png_write_info(png_ptr, info_ptr);
  png_write_image(png_ptr, (png_bytepp)im->pix);
  png_write_end(png_ptr, NULL);
  png_destroy_write_struct(&png_ptr, &info_ptr);

  fclose(pngFile);
  return 0;
}
