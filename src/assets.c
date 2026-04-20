#include <raylib.h>

#include "assets.h"
#include "bullet.h"
#include "powerup.h"

Assets assets;

void load_assets() {
  assets.title_bg = LoadTexture("assets/title.jpg");

  assets.explosion.texture = LoadTexture("assets/explosion.png");
  assets.explosion.frames = 17;

  assets.fire.texture = LoadTexture("assets/fire.png");
  assets.fire.frames = 20;

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

  assets.xp_gems[0].texture = LoadTexture("assets/file.png");
  assets.xp_gems[0].frames = 1;
  assets.xp_gems[1].texture = LoadTexture("assets/files.png");
  assets.xp_gems[1].frames = 1;
  assets.xp_gems[2].texture = LoadTexture("assets/folder.png");
  assets.xp_gems[2].frames = 1;

  assets.enemies[0].texture = LoadTexture("assets/ie.png");
  assets.enemies[0].frames = 1;
  assets.enemies[1].texture = LoadTexture("assets/pain.png");
  assets.enemies[1].frames = 1;
  assets.enemies[2].texture = LoadTexture("assets/phish.png");
  assets.enemies[2].frames = 1;
  assets.enemies[3].texture = LoadTexture("assets/notpad.png");
  assets.enemies[3].frames = 1;

  assets.powerups[POWER_FAST].texture = LoadTexture("assets/powerup_fast.png");
  assets.powerups[POWER_FAST].frames = 1;
  assets.powerups[POWER_DMG].texture = LoadTexture("assets/powerup_dmg.png");
  assets.powerups[POWER_DMG].frames = 1;
}

Rectangle get_frame_rect(Sprite *sprite, u32 frame) {
  return (Rectangle){frame * (f32)sprite->texture.width / sprite->frames, 0,
                     (f32)sprite->texture.width / sprite->frames,
                     sprite->texture.height};
}
