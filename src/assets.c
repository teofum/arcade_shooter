#include <raylib.h>

#include "assets.h"

Assets assets;

void load_assets() {
  assets.explosion.texture = LoadTexture("assets/explosion.png");
  assets.explosion.frames = 17;
}

Rectangle get_frame_rect(Sprite *sprite, u32 frame) {
  return (Rectangle){frame * (f32)sprite->texture.width / sprite->frames, 0,
                     (f32)sprite->texture.width / sprite->frames,
                     sprite->texture.height};
}
