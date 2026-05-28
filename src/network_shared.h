#include <raylib.h>

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

typedef struct Message {
  MessageType type;
  u32 seq;

  union {
    MoveData move;
    ChangeData changes;
    GameState game_state;
  };
} Message;
