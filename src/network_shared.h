#include <raylib.h>

#include "game.h"
#include "types.h"

#define SERVER_PORT 2112

typedef enum {
  MSG_HELLO,
  MSG_GOODBYE,
  MSG_HEARTBEAT,

  MSG_GAME_STATE,
  MSG_MOVE,
} MessageType;

typedef struct EntityMoveData {
  u16 idx;
  Vector2 position;
} EntityMoveData;

typedef struct MoveData {
  EntityMoveData entities[40];
  u8 count;
} MoveData;

typedef struct Message {
  MessageType type;
  u32 seq;

  union {
    MoveData move;
    GameState game_state;
  };
} Message;
