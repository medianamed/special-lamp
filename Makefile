
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -lm
TARGET = image_craft
SRCS = src/main.c src/bmp.c src/filters.c
OBJS = $(SRCS:.c=.o)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: clean
