CC      = emcc
CFLAGS  = -std=c11 -O2 -Ilib -iquote src \
          -s USE_SDL=2 \
          -s FULL_ES3=1 \
          -s ALLOW_MEMORY_GROWTH=1 \
          -s INITIAL_MEMORY=67108864 \
          -DVOXEL_WEB
LDFLAGS = -s USE_SDL=2 \
          -s FULL_ES3=1 \
          -s ALLOW_MEMORY_GROWTH=1 \
          -s INITIAL_MEMORY=67108864 \
          --preload-file assets \
          -s "EXPORTED_RUNTIME_METHODS=['ccall']"
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
          src/ui.c
TARGET  = voxel.html

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

clean:
	rm -f voxel.html voxel.js voxel.wasm voxel.data

.PHONY: all clean
