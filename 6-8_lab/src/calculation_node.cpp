#include "my_zmq.h"
#include <iostream>
#include <map>
#include <unistd.h>

long long node_id;

int main(int argc, char **argv) {
  std::string key;
  int val;
  std::map<std::string, int> dict;
  int rc;
  assert(argc == 2);
  node_id = std::stoll(std::string(argv[1]));

  void *node_parent_context = zmq_ctx_new();
  void *node_parent_socket = zmq_socket(node_parent_context, ZMQ_PAIR);
  rc = zmq_connect(node_parent_socket, ("tcp://localhost:" + std::to_string(PORT_BASE + node_id)).c_str());
  assert(rc == 0);

  long long child_id = -1;
  void *node_context = nullptr;
  void *node_socket = nullptr;
  std::cout << "OK: " << getpid() << std::endl;

  bool has_child = false, awake = true, add = false;
  while (awake) {
	node_token_t token({fail, 0, 0});
	my_zmq::receive_msg(token, node_parent_socket);
	auto *reply = new node_token_t({fail, node_id, node_id});

	if (token.action == bind and token.parent_id == node_id) {
	  /*
			 * Bind could be recieved when parent created node
			 * and this node should bind to parent's child
			 */
	  my_zmq::init_pair_socket(node_context, node_socket);
	  rc = zmq_bind(node_socket, ("tcp://*:" + std::to_string(PORT_BASE + token.id)).c_str());
	  assert(rc == 0);
	  has_child = true;
	  child_id = token.id;
	  auto *token_ping = new node_token_t({ping, child_id, child_id});
	  node_token_t reply_ping({fail, child_id, child_id});
	  if (my_zmq::send_receive_wait(token_ping, reply_ping, node_socket) and reply_ping.action == success) {
		reply->action = success;
	  }
	} else if (token.action == create) {
	  if (token.parent_id == node_id) {
		if (has_child) {
		  rc = zmq_close(node_socket);
		  assert(rc == 0);
		  rc = zmq_ctx_term(node_context);
		  assert(rc == 0);
		}
		my_zmq::init_pair_socket(node_context, node_socket);
		rc = zmq_bind(node_socket, ("tcp://*:" + std::to_string(PORT_BASE + token.id)).c_str());
		assert(rc == 0);
		int fork_id = fork();
		if (fork_id == 0) {
		  rc = execl(NODE_EXECUTABLE_NAME, NODE_EXECUTABLE_NAME, std::to_string(token.id).c_str(), nullptr);
		  assert(rc != -1);
		  return 0;
		} else {
		  bool ok = true;
		  if (has_child) {
			auto *token_bind = new node_token_t({bind, token.id, child_id});
			node_token_t reply_bind({fail, token.id, token.id});
			ok = my_zmq::send_receive_wait(token_bind, reply_bind, node_socket);
			ok = ok and (reply_bind.action == success);
		  }
		  if (ok) {
			/* We should check if child has connected to this node */
			auto *token_ping = new node_token_t({ping, token.id, token.id});
			node_token_t reply_ping({fail, token.id, token.id});
			ok = my_zmq::send_receive_wait(token_ping, reply_ping, node_socket);
			ok = ok and (reply_ping.action == success);
			if (ok) {
			  reply->action = success;
			  child_id = token.id;
			  has_child = true;
			} else {
			  rc = zmq_close(node_socket);
			  assert(rc == 0);
			  rc = zmq_ctx_term(node_context);
			  assert(rc == 0);
			}
		  }
		}
	  } else if (has_child) {
		auto *token_down = new node_token_t(token);
		node_token_t reply_down(token);
		reply_down.action = fail;
		if (my_zmq::send_receive_wait(token_down, reply_down, node_socket) and reply_down.action == success) {
		  *reply = reply_down;
		}
	  }
	} else if (token.action == ping) {
	  if (token.id == node_id) {
		reply->action = success;
	  } else if (has_child) {
		auto *token_down = new node_token_t(token);
		node_token_t reply_down(token);
		reply_down.action = fail;
		if (my_zmq::send_receive_wait(token_down, reply_down, node_socket) and reply_down.action == success) {
		  *reply = reply_down;
		}
	  }
	} else if (token.action == destroy) {
	  if (has_child){
		if (token.id == child_id){
		  bool ok;
		  auto* token_down = new node_token_t({destroy, node_id, child_id});
		  node_token_t reply_down = {fail, child_id, child_id};
		  ok = my_zmq::send_receive_wait(token_down, reply_down, node_socket);
		  if (reply_down.action == destroy){
			rc = zmq_close(node_socket);
			assert(rc == 0);
			rc = zmq_ctx_destroy(node_context);
			assert(rc == 0);
			has_child = false;
			child_id = -1;
		  }
		  else if(reply_down.action == bind){
			rc = zmq_close(node_socket);
			assert(rc == 0);
			rc = zmq_ctx_destroy(node_context);
			assert(rc == 0);
			my_zmq::init_pair_socket(node_context, node_socket);
			rc = zmq_bind(node_socket, ("tcp://*:" + std::to_string(PORT_BASE + reply_down.id)).c_str());
			assert(rc == 0);
			child_id = reply_down.id;
			auto *token_ping = new node_token_t({ping, child_id, child_id});
			node_token_t reply_ping({fail, child_id, child_id});
			ok = my_zmq::send_receive_wait(token_ping, reply_ping, node_socket) and (reply_ping.action == success);
		  }
		  if (ok){
			reply->action = success;
		  }
		} else if (token.id == node_id){
		  rc = zmq_close(node_socket);
		  assert(rc == 0);
		  rc = zmq_ctx_destroy(node_context);
		  assert(rc == 0);
		  awake = false;
		  reply->action = bind;
		  reply->id = child_id;
		  reply->parent_id = token.parent_id;
		} else{
		  auto* token_down = new node_token_t(token);
		  node_token_t reply_down = token;
		  reply_down.action = fail;
		  if (my_zmq::send_receive_wait(token_down, reply_down, node_socket) and (reply_down.action == success)){
			*reply = reply_down;
		  }
		}
	  } else if (token.id == node_id){
		reply->action = destroy;
		awake = false;
	  }
	} else if (token.action == exec_check) {
	  if (token.id == node_id) {
		char c = token.parent_id;
		if (c == SENTINEL) {
		  if (dict.find(key) != dict.end()) {
			std::cout << "OK:" << node_id << ":" << dict[key] << std::endl;
		  } else {
			std::cout << "OK:" << node_id << ":'" << key << "' not found" << std::endl;
		  }
		  reply->action = success;
		  key = "";
		} else {
		  key += c;
		  reply->action = success;
		}
	  } else if (has_child) {
		auto *token_down = new node_token_t(token);
		node_token_t reply_down(token);
		reply_down.action = fail;
		if (my_zmq::send_receive_wait(token_down, reply_down, node_socket) and reply_down.action == success) {
		  *reply = reply_down;
		}
	  }
	} else if (token.action == exec_add) {
	  if (token.id == node_id) {
		char c = token.parent_id;
		if (c == SENTINEL) {
		  add = true;
		  reply->action = success;
		} else if (add) {
		  val = token.parent_id;
		  dict[key] = val;
		  std::cout << "OK:" << node_id << std::endl;
		  add = false;
		  key = "";
		  reply->action = success;
		} else {
		  key += c;
		  reply->action = success;
		}
	  } else if (has_child) {
		auto *token_down = new node_token_t(token);
		node_token_t reply_down(token);
		reply_down.action = fail;
		if (my_zmq::send_receive_wait(token_down, reply_down, node_socket) and reply_down.action == success) {
		  *reply = reply_down;
		}
	  }
	}
	my_zmq::send_msg_no_wait(reply, node_parent_socket);
  }
  rc = zmq_close(node_parent_socket);
  assert(rc == 0);
  rc = zmq_ctx_destroy(node_parent_context);
  assert(rc == 0);
}