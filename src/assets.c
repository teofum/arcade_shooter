#include <raylib.h>

#include "assets.h"
#include "bullet.h"

Assets assets;

void load_assets() {
  assets.explosion.texture = LoadTexture("assets/explosion.png");
  assets.explosion.frames = 17;

  assets.player.texture = LoadTexture("assets/clippy.png");
  assets.player.frames = 1;

  assets.bullets[BULLET_REPLICATE].texture = LoadTexture("assets/cmd.png");
  assets.bullets[BULLET_REPLICATE].frames = 1;
  assets.bullets[BULLET_EXPLOSIVE].texture = LoadTexture("assets/mine.png");
  assets.bullets[BULLET_EXPLOSIVE].frames = 1;
  assets.bullets[BULLET_SHRAPNEL].texture = LoadTexture("assets/recycle.png");
  assets.bullets[BULLET_SHRAPNEL].frames = 1;
  assets.bullets[BULLET_LASER].texture = LoadTexture("assets/internet.png");
  assets.bullets[BULLET_LASER].frames = 1;
  assets.bullets[BULLET_HEALING].texture = LoadTexture("assets/defrag.png");
  assets.bullets[BULLET_HEALING].frames = 1;

  assets.enemy_bullet.texture = LoadTexture("assets/error.png");
  assets.enemy_bullet.frames = 1;
}

Rectangle get_frame_rect(Sprite *sprite, u32 frame) {
  return (Rectangle){frame * (f32)sprite->texture.width / sprite->frames, 0,
                     (f32)sprite->texture.width / sprite->frames,
                     sprite->texture.height};
}
