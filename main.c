#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bmp.h"
#include "filters.h"

int main(int argc, char* argv[]) {
    // Показать справку, если нет аргументов
    if (argc < 3) {
        printf("Использование: %s <входной.bmp> <выходной.bmp> [фильтры]\n", argv[0]);
        printf("Пример: %s input.bmp output.bmp -crop 800 600 -gs -blur 0.5\n", argv[0]);
        printf("Доступные фильтры:\n");
        printf("  -crop width height\n");
        printf("  -gs (серый)\n");
        printf("  -neg (негатив)\n");
        printf("  -sharp (резкость)\n");
        printf("  -edge threshold\n");
        printf("  -med window\n");
        printf("  -blur sigma\n");
        return 1;
    }

    // Загрузить изображение
    Img* img = load_bmp(argv[1]);
    if (!img) {
        printf("Ошибка: не удалось загрузить файл %s\n", argv[1]);
        return 1;
    }

    printf("Загружено изображение: %d x %d пикселей\n", img->w, img->h);

    // Список фильтров
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

    // Применить фильтры по порядку
    int arg_index = 3; // начиная с третьего аргумента (после входного и выходного файлов)
    run_pipeline(img, filters, filter_count, argv, &arg_index);

    // Сохранить результат
    if (save_bmp(argv[2], img)) {
        printf("Успешно сохранено в %s\n", argv[2]);
    } else {
        printf("Ошибка при сохранении файла %s\n", argv[2]);
        free_img(img);
        return 1;
    }

    free_img(img);
    return 0;
}
