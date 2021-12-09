
#ifndef INC_6_8_LAB__ZMQ_H_
#define INC_6_8_LAB__ZMQ_H_

#include <cassert>
#include <cerrno>
#include <cstring>
#include <string>
#include <zmq.hpp>
#include <random>

enum actions_t {
  fail = 0,
  success = 1,
  create = 2,
  destroy = 3,
  bind = 4,
  ping = 5,
  exec_check = 6,
  exec_add = 7
};

const char *NODE_EXECUTABLE_NAME = "calculation_node";
const int PORT_BASE = 8000;
const int WAIT_TIME = 1000;
const char SENTINEL = '$';
struct node_token_t {
  actions_t action;
  long long parent_id, id;
};

namespace my_zmq {
void init_pair_socket(void *&context, void *&socket) {
  int rc;
  context = zmq_ctx_new();
  socket = zmq_socket(context, ZMQ_PAIR);
  rc = zmq_setsockopt(socket, ZMQ_RCVTIMEO, &WAIT_TIME, sizeof(int));
  assert(rc == 0);
  rc = zmq_setsockopt(socket, ZMQ_SNDTIMEO, &WAIT_TIME, sizeof(int));
  assert(rc == 0);
}
template<typename T>
void receive_msg(T &reply_data, void *socket) {
  int rc = 0;
  zmq_msg_t reply;
  zmq_msg_init(&reply);
  rc = zmq_msg_recv(&reply, socket, 0);
  assert(rc == sizeof(T));
  reply_data = *(T *)zmq_msg_data(&reply);
  rc = zmq_msg_close(&reply);
  assert(rc == 0);
}
template<typename T>
bool receive_msg_wait(T &reply_data, void *socket) {
  int rc = 0;
  zmq_msg_t reply;
  zmq_msg_init(&reply);
  rc = zmq_msg_recv(&reply, socket, 0);
  if (rc == -1) {
	zmq_msg_close(&reply);
	return false;
  }
  assert(rc == sizeof(T));
  reply_data = *(T *)zmq_msg_data(&reply);
  rc = zmq_msg_close(&reply);
  assert(rc == 0);
  return true;
}
template<typename T>
void send_msg(T *token, void *socket) {
  int rc = 0;
  zmq_msg_t message;
  zmq_msg_init(&message);
  rc = zmq_msg_init_size(&message, sizeof(T));
  assert(rc == 0);
  rc = zmq_msg_init_data(&message, token, sizeof(T), NULL, NULL);
  assert(rc == 0);
  rc = zmq_msg_send(&message, socket, 0);
  assert(rc == sizeof(T));
}
template<typename T>
bool send_msg_no_wait(T *token, void *socket) {
  int rc;
  zmq_msg_t message;
  zmq_msg_init(&message);
  rc = zmq_msg_init_size(&message, sizeof(T));
  assert(rc == 0);
  rc = zmq_msg_init_data(&message, token, sizeof(T), NULL, NULL);
  assert(rc == 0);
  rc = zmq_msg_send(&message, socket, ZMQ_DONTWAIT);
  if (rc == -1) {
	zmq_msg_close(&message);
	return false;
  }
  assert(rc == sizeof(T));
  return true;
}
/* Returns true if T was successfully queued on the socket */
template<typename T>
bool send_msg_wait(T *token, void *socket) {
  int rc;
  zmq_msg_t message;
  zmq_msg_init(&message);
  rc = zmq_msg_init_size(&message, sizeof(T));
  assert(rc == 0);
  rc = zmq_msg_init_data(&message, token, sizeof(T), NULL, NULL);
  assert(rc == 0);
  rc = zmq_msg_send(&message, socket, 0);
  if (rc == -1) {
	zmq_msg_close(&message);
	return false;
  }
  assert(rc == sizeof(T));
  return true;
}
/* send_msg && receive_msg */
template<typename T>
bool send_receive_wait(T *token_send, T &token_reply, void *socket) {
  if (send_msg_wait(token_send, socket)) {
	if (receive_msg_wait(token_reply, socket)) {
	  return true;
	}
  }
  return false;
}
}// namespace my_zmq

#endif//INC_6_8_LAB__ZMQ_H_
