#include "network_shared.h"

bool has_queued_msgs(MessageQueue *q) {
  return q->delayed_msg_head != q->delayed_msg_tail;
}

u32 get_queued_count(MessageQueue *q) {
  u32 head = q->delayed_msg_head;
  u32 tail = q->delayed_msg_tail;

  if (head <= tail) {
    return tail - head;
  } else {
    return MESSAGE_BUF_SIZE - head + tail;
  }
}

Message *peek_msg(MessageQueue *q) {
  if (has_queued_msgs(q)) {
    return &q->delayed_msg_buffer[q->delayed_msg_head];
  } else {
    return NULL;
  }
}

Message *dequeue_msg(MessageQueue *q) {
  if (has_queued_msgs(q)) {
    Message *msg = &q->delayed_msg_buffer[q->delayed_msg_head];
    q->delayed_msg_head = advance(q->delayed_msg_head);
    return msg;
  } else {
    return NULL;
  }
}

bool enqueue_msg(MessageQueue *q, Message *msg) {
  if (advance(q->delayed_msg_tail) == q->delayed_msg_head) {
    return false;
  } else {
    q->delayed_msg_buffer[q->delayed_msg_tail] = *msg;
    q->delayed_msg_tail = advance(q->delayed_msg_tail);
    return true;
  }
}

/*
 * Returns 1 if the message was dequeued, 0 if the ACK was ignored, -1 on error
 */
i32 ack_msg(MessageQueue *q, u32 seq) {
  Message *queued_msg = peek_msg(q);
  if (queued_msg) {
    if (queued_msg->seq == seq) {
      dequeue_msg(q);
      return 1;
    } else if (queued_msg->seq < seq) {
      return -1;
    }
  }

  return 0;
}

bool is_connected(Connection *c) { return c->last_seen > 0; }
