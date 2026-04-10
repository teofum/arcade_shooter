#ifndef CONFIG_H
#define CONFIG_H

#define WINDOW_WIDTH 1200
#define WINDOW_HEIGHT 800

#define FIELD_HEIGHT 200
#define FIELD_WIDTH 120
#define FIELD_COLS 8
#define GRID_SIZE ((float)FIELD_WIDTH / FIELD_COLS)

#define TARGET_FPS 120

#define PLAYER_SPEED 50.0f
#define PLAYER_ACCEL 0.1f

#define BULLET_SPEED 150.0f

#define ENEMY_SPEED 5.0f

// Time it takes an enemy to move one row, in seconds
#define ROW_TIME (GRID_SIZE / ENEMY_SPEED)

#endif
