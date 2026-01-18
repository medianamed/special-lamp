#include "filters.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif
#include <math.h>
// Вспомогательная функция: ограничить значение от 0 до 255
unsigned char clamp(int val) {
    if (val < 0) return 0;
    if (val > 255) return 255;
    return (unsigned char)val;
}

// 1. Обрезка (-crop width height)
int filter_crop(Img* img, int argc, char** argv) {
    if (argc < 2) return 0; // недостаточно аргументов

    int new_w = atoi(argv[0]);
    int new_h = atoi(argv[1]);

    // Если запрошенное больше — берем всё, что есть
    if (new_w > img->w) new_w = img->w;
    if (new_h > img->h) new_h = img->h;

    // Создаем новое изображение
    Img* new_img = malloc(sizeof(Img));
    new_img->w = new_w;
    new_img->h = new_h;
    new_img->pix = malloc(new_h * sizeof(Pixel*));
    for (int y = 0; y < new_h; y++) {
        new_img->pix[y] = malloc(new_w * sizeof(Pixel));
        for (int x = 0; x < new_w; x++) {
            new_img->pix[y][x] = img->pix[y][x]; // копируем верхний левый угол
        }
    }

    // Заменяем старое изображение
    free_img(img);
    *img = *new_img;
    free(new_img);
    return 1;
}

// 2. Серый оттенок (-gs)
int filter_grayscale(Img* img, int argc, char** argv) {
    for (int y = 0; y < img->h; y++) {
        for (int x = 0; x < img->w; x++) {
            int avg = (img->pix[y][x].r + img->pix[y][x].g + img->pix[y][x].b) / 3;
            img->pix[y][x].r = avg;
            img->pix[y][x].g = avg;
            img->pix[y][x].b = avg;
        }
    }
    return 1;
}

// 3. Негатив (-neg)
int filter_negative(Img* img, int argc, char** argv) {
    for (int y = 0; y < img->h; y++) {
        for (int x = 0; x < img->w; x++) {
            img->pix[y][x].r = 255 - img->pix[y][x].r;
            img->pix[y][x].g = 255 - img->pix[y][x].g;
            img->pix[y][x].b = 255 - img->pix[y][x].b;
        }
    }
    return 1;
}

// 4. Повышение резкости (-sharp)
int filter_sharpen(Img* img, int argc, char** argv) {
    // Матрица для повышения резкости
    int kernel[3][3] = {
        {-1, -1, -1},
        {-1,  9, -1},
        {-1, -1, -1}
    };

    // Создаем временное изображение
    Img* temp = malloc(sizeof(Img));
    temp->w = img->w;
    temp->h = img->h;
    temp->pix = malloc(temp->h * sizeof(Pixel*));
    for (int y = 0; y < temp->h; y++) {
        temp->pix[y] = malloc(temp->w * sizeof(Pixel));
    }

    for (int y = 0; y < img->h; y++) {
        for (int x = 0; x < img->w; x++) {
            int r = 0, g = 0, b = 0;
            for (int dy = -1; dy <= 1; dy++) {
                for (int dx = -1; dx <= 1; dx++) {
                    int ny = y + dy;
                    int nx = x + dx;
                    // Если вышли за границу — берем ближайший пиксель
                    if (ny < 0) ny = 0;
                    if (ny >= img->h) ny = img->h - 1;
                    if (nx < 0) nx = 0;
                    if (nx >= img->w) nx = img->w - 1;

                    r += img->pix[ny][nx].r * kernel[dy+1][dx+1];
                    g += img->pix[ny][nx].g * kernel[dy+1][dx+1];
                    b += img->pix[ny][nx].b * kernel[dy+1][dx+1];
                }
            }
            temp->pix[y][x].r = clamp(r);
            temp->pix[y][x].g = clamp(g);
            temp->pix[y][x].b = clamp(b);
        }
    }

    // Копируем результат обратно
    for (int y = 0; y < img->h; y++) {
        for (int x = 0; x < img->w; x++) {
            img->pix[y][x] = temp->pix[y][x];
        }
    }

    free_img(temp);
    return 1;
}

// 5. Выделение границ (-edge threshold)
int filter_edge(Img* img, int argc, char** argv) {
    if (argc < 1) return 0;

    double threshold = atof(argv[0]); // порог

    // Сначала переводим в серый
    for (int y = 0; y < img->h; y++) {
        for (int x = 0; x < img->w; x++) {
            int avg = (img->pix[y][x].r + img->pix[y][x].g + img->pix[y][x].b) / 3;
            img->pix[y][x].r = avg;
            img->pix[y][x].g = avg;
            img->pix[y][x].b = avg;
        }
    }
Ксения Груздева, [18.01.2026 12:58]
// Матрица для выделения границ
    int kernel[3][3] = {
        {-1, -1, -1},
        {-1,  8, -1},
        {-1, -1, -1}
    };

    Img* temp = malloc(sizeof(Img));
    temp->w = img->w;
    temp->h = img->h;
    temp->pix = malloc(temp->h * sizeof(Pixel*));
    for (int y = 0; y < temp->h; y++) {
        temp->pix[y] = malloc(temp->w * sizeof(Pixel));
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
            // Если выше порога — белый, иначе черный
            if (val > threshold * 255) {
                temp->pix[y][x].r = 255;
                temp->pix[y][x].g = 255;
                temp->pix[y][x].b = 255;
            } else {
                temp->pix[y][x].r = 0;
                temp->pix[y][x].g = 0;
                temp->pix[y][x].b = 0;
            }
        }
    }

    // Копируем результат
    for (int y = 0; y < img->h; y++) {
        for (int x = 0; x < img->w; x++) {
            img->pix[y][x] = temp->pix[y][x];
        }
    }

    free_img(temp);
    return 1;
}

// 6. Медианный фильтр (-med window)
int filter_median(Img* img, int argc, char** argv) {
    if (argc < 1) return 0;
    int win = atoi(argv[0]);
    if (win % 2 == 0) win++; // окно должно быть нечетным

    Img* temp = malloc(sizeof(Img));
    temp->w = img->w;
    temp->h = img->h;
    temp->pix = malloc(temp->h * sizeof(Pixel*));
    for (int y = 0; y < temp->h; y++) {
        temp->pix[y] = malloc(temp->w * sizeof(Pixel));
    }

    for (int y = 0; y < img->h; y++) {
        for (int x = 0; x < img->w; x++) {
            // Собираем значения цветов в окрестности
            int r_vals[win*win], g_vals[win*win], b_vals[win*win];
            int cnt = 0;

            for (int dy = -(win/2); dy <= win/2; dy++) {
                for (int dx = -(win/2); dx <= win/2; dx++) {
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

            // Сортируем и берем медиану (среднее значение)
            for (int i = 0; i < cnt-1; i++) {
                for (int j = i+1; j < cnt; j++) {
                    if (r_vals[i] > r_vals[j]) { int t = r_vals[i]; r_vals[i] = r_vals[j]; r_vals[j] = t; }
                    if (g_vals[i] > g_vals[j]) { int t = g_vals[i]; g_vals[i] = g_vals[j]; g_vals[j] = t; }
                    if (b_vals[i] > b_vals[j]) { int t = b_vals[i]; b_vals[i] = b_vals[j]; b_vals[j] = t; }
                }
            }

            int mid = cnt / 2;
            temp->pix[y][x].r = r_vals[mid];
            temp->pix[y][x].g = g_vals[mid];
            temp->pix[y][x].b = b_vals[mid];
        }
    }

    // Копируем результат
    for (int y = 0; y < img->h; y++) {
        for (int x = 0; x < img->w; x++) {
            img->pix[y][x] = temp->pix[y][x];
        }
    }

    free_img(temp);
    return 1;
}

// 7. Гауссово размытие (-blur sigma)
int filter_blur(Img* img, int argc, char** argv) {
    if (argc < 1) return 0;
    double sigma = atof(argv[0]);
    if (sigma <= 0) sigma = 1.0;

    // Размер ядра — 6*sigma, но не меньше 3
    int radius = (int)(sigma * 3);
    if (radius < 1) radius = 1;
    int size = 2 * radius + 1;

Ксения Груздева, [18.01.2026 12:58]
// Создаем ядро
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

    // Нормализуем ядро
    for (int i = 0; i < size*size; i++) kernel[i] /= sum;

    Img* temp = malloc(sizeof(Img));
    temp->w = img->w;
    temp->h = img->h;
    temp->pix = malloc(temp->h * sizeof(Pixel*));
    for (int y = 0; y < temp->h; y++) {
        temp->pix[y] = malloc(temp->w * sizeof(Pixel));
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

                    double weight = kernel[(dy+radius)*size + (dx+radius)];
                    r += img->pix[ny][nx].r * weight;
                    g += img->pix[ny][nx].g * weight;
                    b += img->pix[ny][nx].b * weight;
                }
            }
            temp->pix[y][x].r = clamp((int)r);
            temp->pix[y][x].g = clamp((int)g);
            temp->pix[y][x].b = clamp((int)b);
        }
    }

    // Копируем результат
    for (int y = 0; y < img->h; y++) {
        for (int x = 0; x < img->w; x++) {
            img->pix[y][x] = temp->pix[y][x];
        }
    }

    free_img(temp);
    free(kernel);
    return 1;
}

// Контроллер — проходим по всем аргументам и применяем фильтры
void run_pipeline(Img* img, Filter* filters, int count, char** args, int* index) {
    while (args[*index]) {
        char* arg = args[*index];
        if (arg[0] != '-') {
            (*index)++;
            continue; // пропускаем не фильтры
        }

        int found = 0;
        for (int i = 0; i < count; i++) {
            if (strcmp(arg, filters[i].name) == 0) {
                found = 1;
                (*index)++; // переходим к следующему аргументу

                // Проверяем, хватает ли параметров
                int needed = filters[i].params_count;
                int available = 0;
                for (int j = *index; j < count && args[j] && args[j][0] != '-'; j++) {
                    available++;
                }

                if (available < needed) {
                    printf("Ошибка: фильтр %s требует %d параметров, передано %d\n", filters[i].name, needed, available);
                    break;
                }

                // Вызываем фильтр
                char** params = &args[*index];
                if (filters[i].apply(img, needed, params)) {
                    printf("Фильтр %s применен\n", filters[i].name);
                } else {
                    printf("Ошибка при применении фильтра %s\n", filters[i].name);
                }

                (*index) += needed; // пропускаем использованные параметры
                break;
            }
        }

        if (!found) {
            printf("Неизвестный фильтр: %s\n", arg);
            (*index)++;
        }
    }
}
