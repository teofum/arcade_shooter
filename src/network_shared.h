#include <raylib.h>

#include "entity.h"
#include "entity_list.h"
#include "game.h"
#include "types.h"

#define SERVER_PORT 2112
#define MOVE_ENT_COUNT 40
#define CHANGE_ENT_COUNT 12

typedef enum {
  MSG_HELLO,
  MSG_GOODBYE,
  MSG_HEARTBEAT,

  MSG_GAME_STATE,
  MSG_MOVE,
  MSG_CHANGES,
} MessageType;

typedef struct EntityMoveData {
  u16 idx;
  Vector2 position;
} EntityMoveData;

typedef struct MoveData {
  EntityMoveData entities[MOVE_ENT_COUNT];
  u8 count;
} MoveData;

typedef struct ChangeData {
  EntityListChange changes[CHANGE_ENT_COUNT];
  u8 count;
} ChangeData;

typedef struct GameStateData {
  GameState state;

  f32 total_time;
  f32 delta_time;

  u32 boss_idx;
  f32 boss_timer;

  u32 score;

  u32 menu_selected_option;
  u32 menu_n_options;
  Orientation menu_layout;

  Player player_state;
} GameStateData;

typedef struct Message {
  MessageType type;
  u32 seq;

  union {
    MoveData move;
    ChangeData changes;
    GameStateData game;
  };
} Message;
