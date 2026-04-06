CC      = gcc
CFLAGS  = -std=c11 -O2 -Wall -Wextra -Ilib/glad/include -Ilib -Isrc
LDFLAGS = $(shell sdl2-config --libs) -lGL -lm
SRCS    = src/main.c \
          src/platform.c \
          src/input.c \
          src/blocks.c \
          src/chunk.c \
          src/world.c \
          src/worldgen.c \
          src/mesh.c \
          src/physics.c \
          src/player.c \
          src/renderer.c \
          src/minimap.c \
          src/ui.c \
          lib/glad/src/gl.c
TARGET  = voxel

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(shell sdl2-config --cflags) $^ -o $@ $(LDFLAGS)

clean:
	rm -f $(TARGET)

.PHONY: all clean
