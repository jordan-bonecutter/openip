#include "openip.h"

int main()
{
  img_ioerr e;
  img* i = img_from_file("img/foo.jpg", &e, true, 100);
  i->pix[10][10] = (pixel){0xff, 0x00, 0x00, 0xff};
  e = img_save_to_file(i, "img/foo2.png", true, 100);
  img_fftransform(i);
  img_free(i);
  return 0;
}
