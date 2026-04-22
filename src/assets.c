#include <raylib.h>

#include "assets.h"
#include "bullet.h"
#include "powerup.h"

Assets assets;

static Sprite load_sprite(const char *filename, u32 frames) {
  Sprite s = {
      .texture = LoadTexture(filename),
      .frames = frames,
  };

  return s;
}

void load_assets() {
  assets.title_bg = LoadTexture("assets/title.jpg");

  assets.explosion = load_sprite("assets/explosion.png", 17);
  assets.fire = load_sprite("assets/fire.png", 20);

  assets.player = load_sprite("assets/clippy.png", 1);

  assets.bullets[BULLET_REPLICATE] = load_sprite("assets/cmd.png", 1);
  assets.bullets[BULLET_EXPLOSIVE] = load_sprite("assets/mine.png", 1);
  assets.bullets[BULLET_SHRAPNEL] = load_sprite("assets/recycle.png", 1);
  assets.bullets[BULLET_LASER] = load_sprite("assets/internet.png", 1);
  assets.bullets[BULLET_HEALING] = load_sprite("assets/defrag.png", 1);

  assets.enemy_bullet = load_sprite("assets/error.png", 1);

  assets.xp_gems[0] = load_sprite("assets/file.png", 1);
  assets.xp_gems[1] = load_sprite("assets/files.png", 1);
  assets.xp_gems[2] = load_sprite("assets/folder.png", 1);

  assets.enemies[0] = load_sprite("assets/ie.png", 1);
  assets.enemies[1] = load_sprite("assets/pain.png", 1);
  assets.enemies[2] = load_sprite("assets/phish.png", 1);
  assets.enemies[3] = load_sprite("assets/notpad.png", 1);

  assets.boss_chungus[0] = load_sprite("assets/miniboss1.png", 1);
  assets.boss_chungus[1] = load_sprite("assets/miniboss1_dmg.png", 1);
  assets.boss_tank[0] = load_sprite("assets/tank.png", 1);
  assets.boss_tank[1] = load_sprite("assets/tank_dmg.png", 1);
  assets.boss_tank_gun = load_sprite("assets/tank_gun.png", 1);

  assets.boss_final[0] = load_sprite("assets/boss.png", 1);
  assets.boss_final[1] = load_sprite("assets/boss_dmg_1.png", 1);
  assets.boss_final[2] = load_sprite("assets/boss_dmg_2.png", 1);

  assets.powerups[POWER_FAST] = load_sprite("assets/powerup_fast.png", 1);
  assets.powerups[POWER_DMG] = load_sprite("assets/powerup_dmg.png", 1);

  assets.sfx_hit = LoadSound("assets/sfx_hit.wav");
  assets.sfx_explosion = LoadSound("assets/sfx_explosion.wav");
  assets.sfx_oof = LoadSound("assets/sfx_oof.wav");
}

Rectangle get_frame_rect(Sprite *sprite, u32 frame) {
  return (Rectangle){frame * (f32)sprite->texture.width / sprite->frames, 0,
                     (f32)sprite->texture.width / sprite->frames,
                     sprite->texture.height};
}
