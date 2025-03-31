CC = gcc
LDFLAGS := -lm -lX11 -lpthread
CFLAGS := -g -O2 -I./gifdec

OBJECT_DIR = obj
SRC=$(wildcard *.c) $(wildcard gifdec/*.c)

OBJ = $(patsubst %.c,$(OBJECT_DIR)/%.o,$(SRC))

xinerama ?= 1

ifeq (${xinerama},1)
	CFLAGS += -DHAVE_LIBXINERAMA
	LDFLAGS += -lXinerama
endif

all: gifpaper

gifpaper: $(OBJ)
	$(CC) -o gifpaper $^ $(LDFLAGS)

$(OBJECT_DIR)/%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -rf gifpaper obj/* *.inc

install: gifpaper
	@install -Dm755 gifpaper $(DESTDIR)/usr/bin/gifpaper
