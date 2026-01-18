
#include "bmp.h"
#include <string.h>

#pragma pack(1)
typedef struct {
    unsigned short type;      // 'BM'
    unsigned int size;        // размер файла
    unsigned short reserved1, reserved2;
    unsigned int offset;      // смещение до данных
} FileHeader;

typedef struct {
    unsigned int size;        // размер заголовка
    int width, height;        // ширина и высота
    unsigned short planes;    // всегда 1
    unsigned short bit_count; // бит на пиксель (24)
    unsigned int compression; // 0 = нет
    unsigned int image_size;  // размер изображения
    int x_res, y_res;         // разрешение
    unsigned int colors_used;
    unsigned int colors_important;
} InfoHeader;
#pragma pack()

Img* load_bmp(const char* fname) {
    FILE* f = fopen(fname, "rb");
    if (!f) return NULL;

    FileHeader fh;
    InfoHeader ih;

    fread(&fh, sizeof(FileHeader), 1, f);
    if (fh.type != 0x4D42) { fclose(f); return NULL; } // Не BMP

    fread(&ih, sizeof(InfoHeader), 1, f);
    if (ih.bit_count != 24 || ih.compression != 0) {
        fclose(f); return NULL; // Неподдерживаемый формат
    }

    Img* img = malloc(sizeof(Img));
    img->w = ih.width;
    img->h = ih.height;

    // Выделяем память под пиксели
    img->pix = malloc(img->h * sizeof(Pixel*));
    for (int y = 0; y < img->h; y++) {
        img->pix[y] = malloc(img->w * sizeof(Pixel));
    }

    // Читаем данные (BGR, строки в обратном порядке)
    int pad = (4 - (img->w * 3) % 4) % 4; // padding в конце строки
    for (int y = img->h - 1; y >= 0; y--) {
        for (int x = 0; x < img->w; x++) {
            unsigned char bgr[3];
            fread(bgr, 1, 3, f);
            img->pix[y][x].b = bgr[0];
            img->pix[y][x].g = bgr[1];
            img->pix[y][x].r = bgr[2];
        }
        fseek(f, pad, SEEK_CUR); // пропускаем padding
    }

    fclose(f);
    return img;
}

int save_bmp(const char* fname, Img* img) {
    FILE* f = fopen(fname, "wb");
    if (!f) return 0;

    FileHeader fh = {0};
    InfoHeader ih = {0};

    fh.type = 0x4D42;
    fh.size = 54 + img->w * img->h * 3 + (img->w * 3) % 4 * img->h;
    fh.offset = 54;

    ih.size = 40;
    ih.width = img->w;
    ih.height = img->h;
    ih.planes = 1;
    ih.bit_count = 24;
    ih.compression = 0;
    ih.image_size = img->w * img->h * 3;
    ih.x_res = 2835;
    ih.y_res = 2835;

    fwrite(&fh, sizeof(FileHeader), 1, f);
    fwrite(&ih, sizeof(InfoHeader), 1, f);

    int pad = (4 - (img->w * 3) % 4) % 4;
    for (int y = img->h - 1; y >= 0; y--) {
        for (int x = 0; x < img->w; x++) {
            unsigned char bgr[3] = {
                img->pix[y][x].b,
                img->pix[y][x].g,
                img->pix[y][x].r
            };
            fwrite(bgr, 1, 3, f);
        }
        for (int k = 0; k < pad; k++) fputc(0, f);
    }

    fclose(f);
    return 1;
}

void free_img(Img* img) {
    if (!img) return;
    for (int y = 0; y < img->h; y++) free(img->pix[y]);
    free(img->pix);
    free(img);
}
