#ifndef FILTERS_H
#define FILTERS_H

#include "bmp.h"

// Тип функции фильтра
typedef int (*FilterFunc)(Img*, int, char**);

// Структура фильтра
typedef struct {
    const char* name;     // имя фильтра, например "-gs"
    FilterFunc apply;     // функция применения
    int params_count;     // сколько параметров ожидает фильтр
} Filter;

// Объявления фильтров
int filter_crop(Img* img, int argc, char** argv);
int filter_grayscale(Img* img, int argc, char** argv);
int filter_negative(Img* img, int argc, char** argv);
int filter_sharpen(Img* img, int argc, char** argv);
int filter_edge(Img* img, int argc, char** argv);
int filter_median(Img* img, int argc, char** argv);
int filter_blur(Img* img, int argc, char** argv);

// Контроллер — применяем все фильтры по порядку
void run_pipeline(Img* img, Filter* filters, int count, char** args, int* index);

#endif
