/* * * * * * * * * * * * * * * * * * * * * * * * * * */
/* openip_globals.h  * * * * * * * * * * * * * * * * */
/* created by: jordan bonecutter * * * * * * * * * * */
/* * * * * * * * * * * * * * * * * * * * * * * * * * */
/* * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef __OPENIP_GLOBALS_H__
#define __OPENIP_GLOBALS_H__

#include <stdint.h>
#include <stdbool.h>

typedef uint8_t gray_t;
typedef struct{gray_t r, g, b, a;} pixel;
typedef struct{pixel** pix; int width, height;} img;

#endif
