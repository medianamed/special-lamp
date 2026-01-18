// filters.h
#ifndef FILTERS_H
#define FILTERS_H

#include "bmp.h"

typedef int (*FilterFunc)(Img*, int, char**);

typedef struct {
    const char* name;
    FilterFunc apply;
    int params_count;
} Filter;

int filter_crop(Img* img, int argc, char** argv);
int filter_grayscale(Img* img, int argc, char** argv);
int filter_negative(Img* img, int argc, char** argv);
int filter_sharpen(Img* img, int argc, char** argv);
int filter_edge(Img* img, int argc, char** argv);
int filter_median(Img* img, int argc, char** argv);
int filter_blur(Img* img, int argc, char** argv);

void run_pipeline(Img* img, Filter* filters, int count, char** args, int* index);

#endif

