#ifndef ASSETS_H
#define ASSETS_H

#include <raylib.h>

#include "types.h"

typedef struct Sprite {
  Texture2D texture;
  i32 frames;
} Sprite;

typedef struct Assets {
  Sprite explosion;
} Assets;

extern Assets assets;

void load_assets();

Rectangle get_frame_rect(Sprite *sprite, u32 frame);

#endif
