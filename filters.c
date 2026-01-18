// filters.c
#include "filters.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

unsigned char clamp(int val) {
    if (val < 0) return 0;
    if (val > 255) return 255;
    return (unsigned char)val;
}

// 1. Crop (-crop width height)
int filter_crop(Img* img, int argc, char** argv) {
    (void)argc;
    (void)argv;

    if (argc < 2) return 0;

    int new_w = atoi(argv[0]);
    int new_h = atoi(argv[1]);

    if (new_w <= 0 || new_h <= 0) return 0;
    if (new_w > img->w) new_w = img->w;
    if (new_h > img->h) new_h = img->h;

    Pixel** new_pix = malloc(new_h * sizeof(Pixel*));
    for (int y = 0; y < new_h; y++) {
        new_pix[y] = malloc(new_w * sizeof(Pixel));
        for (int x = 0; x < new_w; x++) {
            new_pix[y][x] = img->pix[y][x];
        }
    }

    for (int y = 0; y < img->h; y++) {
        free(img->pix[y]);
    }
    free(img->pix);

    img->w = new_w;
    img->h = new_h;
    img->pix = new_pix;

    return 1;
}

// 2. Grayscale (-gs)
int filter_grayscale(Img* img, int argc, char** argv) {
    (void)argc;
    (void)argv;

    for (int y = 0; y < img->h; y++) {
        for (int x = 0; x < img->w; x++) {
            int avg = (img->pix[y][x].r + img->pix[y][x].g + img->pix[y][x].b) / 3;
            img->pix[y][x].r = img->pix[y][x].g = img->pix[y][x].b = avg;
        }
    }
    return 1;
}

// 3. Negative (-neg)
int filter_negative(Img* img, int argc, char** argv) {
    (void)argc;
    (void)argv;

    for (int y = 0; y < img->h; y++) {
        for (int x = 0; x < img->w; x++) {
            img->pix[y][x].r = 255 - img->pix[y][x].r;
            img->pix[y][x].g = 255 - img->pix[y][x].g;
            img->pix[y][x].b = 255 - img->pix[y][x].b;
        }
    }
    return 1;
}

// 4. Sharpen (-sharp)
int filter_sharpen(Img* img, int argc, char** argv) {
    (void)argc;
    (void)argv;

    int kernel[3][3] = {
        {-1, -1, -1},
        {-1,  9, -1},
        {-1, -1, -1}
    };

    Img temp = { .w = img->w, .h = img->h };
    temp.pix = malloc(temp.h * sizeof(Pixel*));
    for (int y = 0; y < temp.h; y++) {
        temp.pix[y] = malloc(temp.w * sizeof(Pixel));
    }

    for (int y = 0; y < img->h; y++) {
        for (int x = 0; x < img->w; x++) {
            int r = 0, g = 0, b = 0;
            for (int dy = -1; dy <= 1; dy++) {
                for (int dx = -1; dx <= 1; dx++) {
                    int ny = y + dy;
                    int nx = x + dx;
                    if (ny < 0) ny = 0;
                    if (ny >= img->h) ny = img->h - 1;
                    if (nx < 0) nx = 0;
                    if (nx >= img->w) nx = img->w - 1;

                    int k = kernel[dy+1][dx+1];
                    r += img->pix[ny][nx].r * k;
                    g += img->pix[ny][nx].g * k;
                    b += img->pix[ny][nx].b * k;
                }
            }
            temp.pix[y][x].r = clamp(r);
            temp.pix[y][x].g = clamp(g);
            temp.pix[y][x].b = clamp(b);
        }
    }

    for (int y = 0; y < img->h; y++) {
        for (int x = 0; x < img->w; x++) {
            img->pix[y][x] = temp.pix[y][x];
        }
        free(temp.pix[y]);
    }
    free(temp.pix);
    return 1;
}

// 5. Edge detection (-edge threshold)
int filter_edge(Img* img, int argc, char** argv) {
    if (argc < 1) return 0;
    double threshold = atof(argv[0]);

    // Convert to grayscale
    filter_grayscale(img, 0, NULL);

    int kernel[3][3] = {
        {-1, -1, -1},
        {-1,  8, -1},
        {-1, -1, -1}
    };

    Img temp = { .w = img->w, .h = img->h };
    temp.pix = malloc(temp.h * sizeof(Pixel*));
    for (int y = 0; y < temp.h; y++) {
        temp.pix[y] = malloc(temp.w * sizeof(Pixel));
    }

    for (int y = 0; y < img->h; y++) {
        for (int x = 0; x < img->w; x++) {
            int sum = 0;
            for (int dy = -1; dy <= 1; dy++) {
                for (int dx = -1; dx <= 1; dx++) {
                    int ny = y + dy;
                    int nx = x + dx;
                    if (ny < 0) ny = 0;
                    if (ny >= img->h) ny = img->h - 1;
                    if (nx < 0) nx = 0;
                    if (nx >= img->w) nx = img->w - 1;

                    sum += img->pix[ny][nx].r * kernel[dy+1][dx+1];
                }
            }
            int val = clamp(sum);
            unsigned char out = (val > threshold * 255) ? 255 : 0;
            temp.pix[y][x].r = temp.pix[y][x].g = temp.pix[y][x].b = out;
        }
    }

    for (int y = 0; y < img->h; y++) {
        for (int x = 0; x < img->w; x++) {
            img->pix[y][x] = temp.pix[y][x];
        }
        free(temp.pix[y]);
    }
    free(temp.pix);
    return 1;
}

// 6. Median filter (-med window)
int filter_median(Img* img, int argc, char** argv) {
    if (argc < 1) return 0;
    int win = atoi(argv[0]);
    if (win % 2 == 0) win++;
    if (win < 1) win = 1;

    Img temp = { .w = img->w, .h = img->h };
    temp.pix = malloc(temp.h * sizeof(Pixel*));
    for (int y = 0; y < temp.h; y++) {
        temp.pix[y] = malloc(temp.w * sizeof(Pixel));
    }

    int radius = win / 2;
    for (int y = 0; y < img->h; y++) {
        for (int x = 0; x < img->w; x++) {
            int r_vals[win*win], g_vals[win*win], b_vals[win*win];
            int cnt = 0;

            for (int dy = -radius; dy <= radius; dy++) {
                for (int dx = -radius; dx <= radius; dx++) {
                    int ny = y + dy;
                    int nx = x + dx;
                    if (ny < 0) ny = 0;
                    if (ny >= img->h) ny = img->h - 1;
                    if (nx < 0) nx = 0;
                    if (nx >= img->w) nx = img->w - 1;

                    r_vals[cnt] = img->pix[ny][nx].r;
                    g_vals[cnt] = img->pix[ny][nx].g;
                    b_vals[cnt] = img->pix[ny][nx].b;
                    cnt++;
                }
            }

            for (int i = 0; i < cnt; i++) {
                for (int j = i + 1; j < cnt; j++) {
                    if (r_vals[i] > r_vals[j]) { int t = r_vals[i]; r_vals[i] = r_vals[j]; r_vals[j] = t; }
                    if (g_vals[i] > g_vals[j]) { int t = g_vals[i]; g_vals[i] = g_vals[j]; g_vals[j] = t; }
                    if (b_vals[i] > b_vals[j]) { int t = b_vals[i]; b_vals[i] = b_vals[j]; b_vals[j] = t; }
                }
            }

            int mid = cnt / 2;
            temp.pix[y][x].r = r_vals[mid];
            temp.pix[y][x].g = g_vals[mid];
            temp.pix[y][x].b = b_vals[mid];
        }
    }

    for (int y = 0; y < img->h; y++) {
        for (int x = 0; x < img->w; x++) {
            img->pix[y][x] = temp.pix[y][x];
        }
        free(temp.pix[y]);
    }
    free(temp.pix);
    return 1;
}

// 7. Gaussian blur (-blur sigma)
int filter_blur(Img* img, int argc, char** argv) {
    if (argc < 1) return 0;
    double sigma = atof(argv[0]);
    if (sigma <= 0) sigma = 1.0;

    int radius = (int)(sigma * 3);
    if (radius < 1) radius = 1;
    int size = 2 * radius + 1;

    double* kernel = malloc(size * size * sizeof(double));
    double sum = 0.0;

    for (int dy = -radius; dy <= radius; dy++) {
        for (int dx = -radius; dx <= radius; dx++) {
            double dist = dy*dy + dx*dx;
            double val = exp(-dist / (2 * sigma * sigma)) / (2 * M_PI * sigma * sigma);
            kernel[(dy+radius)*size + (dx+radius)] = val;
            sum += val;
        }
    }

    for (int i = 0; i < size*size; i++) {
        kernel[i] /= sum;
    }

    Img temp = { .w = img->w, .h = img->h };
    temp.pix = malloc(temp.h * sizeof(Pixel*));
    for (int y = 0; y < temp.h; y++) {
        temp.pix[y] = malloc(temp.w * sizeof(Pixel));
    }

    for (int y = 0; y < img->h; y++) {
        for (int x = 0; x < img->w; x++) {
            double r = 0.0, g = 0.0, b = 0.0;
            for (int dy = -radius; dy <= radius; dy++) {
                for (int dx = -radius; dx <= radius; dx++) {
                    int ny = y + dy;
                    int nx = x + dx;
                    if (ny < 0) ny = 0;
                    if (ny >= img->h) ny = img->h - 1;
                    if (nx < 0) nx = 0;
                    if (nx >= img->w) nx = img->w - 1;

                    double w = kernel[(dy+radius)*size + (dx+radius)];
                    r += img->pix[ny][nx].r * w;
                    g += img->pix[ny][nx].g * w;
                    b += img->pix[ny][nx].b * w;
                }
            }
            temp.pix[y][x].r = clamp((int)r);
            temp.pix[y][x].g = clamp((int)g);
            temp.pix[y][x].b = clamp((int)b);
        }
    }

    for (int y = 0; y < img->h; y++) {
        for (int x = 0; x < img->w; x++) {
            img->pix[y][x] = temp.pix[y][x];
        }
        free(temp.pix[y]);
    }
    free(temp.pix);
    free(kernel);
    return 1;
}

// Apply filters in sequence
void run_pipeline(Img* img, Filter* filters, int count, char** args, int* index) {
    while (args[*index]) {
        char* arg = args[*index];
        if (arg[0] != '-') {
            (*index)++;
            continue;
        }

        int found = 0;
        for (int i = 0; i < count; i++) {
            if (strcmp(arg, filters[i].name) == 0) {
                found = 1;
                (*index)++; // move to parameters

                int available = 0;
                int j = *index;
                while (args[j] && args[j][0] != '-') {
                    available++;
                    j++;
                }

                int needed = filters[i].params_count;
                if (available < needed) {
                    printf("Error: filter %s requires %d parameters, got %d\n",
                           filters[i].name, needed, available);
                    break;
                }

                char** params = &args[*index];
                if (filters[i].apply(img, needed, params)) {
                    printf("Filter %s applied\n", filters[i].name);
                } else {
                    printf("Error applying filter %s\n", filters[i].name);
                }

                (*index) += needed;
                break;
            }
        }

        if (!found) {
            printf("Unknown filter: %s\n", arg);
            (*index)++;
        }
    }
}
