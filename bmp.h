
#ifndef BMP_H
#define BMP_H

#include <stdio.h>
#include <stdlib.h>

// Один пиксель — красный, зеленый, синий
typedef struct {
    unsigned char r, g, b;
} Pixel;

// Изображение — ширина, высота, двумерный массив пикселей
typedef struct {
    int w, h;           // ширина и высота
    Pixel** pix;        // массив пикселей: pix[y][x]
} Img;

// Функции для работы с BMP
Img* load_bmp(const char* fname);
int save_bmp(const char* fname, Img* img);
void free_img(Img* img);

#endif
