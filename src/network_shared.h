#include <raylib.h>

#include "entity.h"
#include "entity_list.h"
#include "game.h"
#include "types.h"

#define SERVER_PORT 2112
#define MOVE_ENT_COUNT 40
#define CHANGE_ENT_COUNT 12
#define UPDATE_ENT_COUNT 8

#define MESSAGE_BUF_SIZE 128
#define advance(x) ((x) == MESSAGE_BUF_SIZE - 1 ? 0 : (x) + 1)

#define ENABLE_PACKET_LOSS 0
#define PACKET_LOSS_RATE 0.3

typedef enum {
  MSG_HELLO,
  MSG_GOODBYE,
  MSG_HEARTBEAT,
  MSG_ACK,

  MSG_GAME_STATE,
  MSG_MOVE,
  MSG_CHANGES,
  MSG_UPDATE,

  MSG_END_GAME,
  MSG_RESET,
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

typedef struct EntityUpdateData {
  u16 idx;

  union {
    Enemy enemy;
  };
} EntityUpdateData;

typedef struct UpdateData {
  EntityUpdateData updates[UPDATE_ENT_COUNT];
  u8 count;
} UpdateData;

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
  bool priority;

  union {
    MoveData move;
    ChangeData changes;
    UpdateData updates;
    GameStateData game;
  };
} Message;

typedef struct MessageQueue {
  Message delayed_msg_buffer[MESSAGE_BUF_SIZE];
  u32 delayed_msg_head;
  u32 delayed_msg_tail;
} MessageQueue;

bool has_queued_msgs(MessageQueue *q);
u32 get_queued_count(MessageQueue *q);
Message *peek_msg(MessageQueue *q);
Message *dequeue_msg(MessageQueue *q);
bool enqueue_msg(MessageQueue *q, Message *msg);
i32 ack_msg(MessageQueue *q, u32 seq);
