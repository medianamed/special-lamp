gcc -std=c99 -Wall -Wextra -o bmpedit main.c filters.c bmp.c -lm

.\bmpedit.exe input.bmp output.bmp -gs
