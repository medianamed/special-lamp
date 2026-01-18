// bmp.h
#ifndef BMP_H
#define BMP_H

#include <stdio.h>
#include <stdlib.h>

typedef struct {
    unsigned char r, g, b;
} Pixel;

typedef struct {
    int w, h;
    Pixel** pix;
} Img;

Img* load_bmp(const char* fname);
int save_bmp(const char* fname, Img* img);
void free_img(Img* img);

#endif

