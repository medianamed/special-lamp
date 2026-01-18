// main.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bmp.h"
#include "filters.h"

int main(int argc, char* argv[]) {
    if (argc < 3) {
        printf("Usage: %s <input.bmp> <output.bmp> [filters]\n", argv[0]);
        printf("Example: %s input.bmp output.bmp -crop 800 600 -gs -blur 0.5\n", argv[0]);
        printf("Available filters:\n");
        printf("  -crop width height\n");
        printf("  -gs (grayscale)\n");
        printf("  -neg (negative)\n");
        printf("  -sharp (sharpen)\n");
        printf("  -edge threshold\n");
        printf("  -med window\n");
        printf("  -blur sigma\n");
        return 1;
    }

    Img* img = load_bmp(argv[1]);
    if (!img) {
        printf("Error: failed to load file %s\n", argv[1]);
        return 1;
    }

    printf("Loaded image: %d x %d pixels\n", img->w, img->h);

    Filter filters[] = {
        {"-crop", filter_crop, 2},
        {"-gs", filter_grayscale, 0},
        {"-neg", filter_negative, 0},
        {"-sharp", filter_sharpen, 0},
        {"-edge", filter_edge, 1},
        {"-med", filter_median, 1},
        {"-blur", filter_blur, 1}
    };
    int filter_count = sizeof(filters) / sizeof(Filter);

    int arg_index = 3;
    run_pipeline(img, filters, filter_count, argv, &arg_index);

    if (save_bmp(argv[2], img)) {
        printf("Saved successfully to %s\n", argv[2]);
    } else {
        printf("Error saving file %s\n", argv[2]);
        free_img(img);
        return 1;
    }

    free_img(img);
    return 0;
}
