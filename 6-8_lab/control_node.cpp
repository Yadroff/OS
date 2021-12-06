#include <unistd.h>
#include <vector>
#include <zmq.hpp>

#include "my_zmq.h"
#include "topology.h"

using node_id_type = long long;

int main() {
  int rc;
  topology_t control_node;
  std::vector<std::pair<void *, void *>> children;
  std::string s;
  node_id_type id;
  while (std::cin >> s >> id) {
	if (s == "create") {
	  node_id_type parent;
	  std::cin >> parent;
	  int ind;
	  if (parent == -1) {
		void *new_context = nullptr;
		void *new_socket = nullptr;
		my_zmq::init_pair_socket(new_context, new_socket);
		rc = zmq_bind(new_socket, ("tcp://*:" + std::to_string(PORT_BASE + id)).c_str());
		assert(rc == 0);
		int child_id = fork();
		if (child_id == 0) {//child
		  execl(NODE_EXECUTABLE_NAME, NODE_EXECUTABLE_NAME, std::to_string(id).c_str(), NULL);
		} else {
		  auto *token = new node_token_t({ping, id, id});
		  node_token_t reply({fail, id, id});
		  if (my_zmq::send_receive_wait(token, reply, new_socket) && reply.action == success) {
			children.emplace_back(std::make_pair(new_context, new_socket));
			control_node.insert(id);
		  } else {
			assert(zmq_close(new_socket) == 0);
			assert(zmq_ctx_term(new_context) == 0);
		  }
		}
	  } else if ((ind = control_node.find(parent)) == -1) {
		std::cout << "Error: Not found" << std::endl;
	  } else {
		if (control_node.find(id)) {
		  std::cout << "Error: Already exists" << std::endl;
		  continue;
		}
		node_token_t *token = new node_token_t({create, parent, id});
		node_token_t reply({fail, id, id});
		if (my_zmq::send_receive_wait(token, reply, children[ind].second) and reply.action == success) {
		  control_node.insert(parent, id);
		} else {
		  std::cout << "Error: Parent is unavailable" << std::endl;
		}
	  }
	} else if (s == "remove") {
	  int ind = control_node.find(id);
	  if (ind != -1) {
		auto *token = new node_token_t({destroy, id, id});
		node_token_t reply({fail, id, id});
		bool ok = my_zmq::send_receive_wait(token, reply, children[ind].second);
		if (reply.action == destroy and reply.parent_id == id) {
		  rc = zmq_close(children[ind].second);
		  assert(rc == 0);
		  rc = zmq_ctx_term(children[ind].first);
		  assert(rc == 0);
		  auto it = children.begin();
		  while (ind--) {
			++it;
		  }
		  children.erase(it);
		} else if (reply.action == bind and reply.parent_id == id) {
		  rc = zmq_close(children[ind].second);
		  assert(rc == 0);
		  rc = zmq_ctx_term(children[ind].first);
		  assert(rc == 0);
		  my_zmq::init_pair_socket(children[ind].first, children[ind].second);
		  rc = zmq_bind(children[ind].second, ("tcp://*:" + std::to_string(PORT_BASE + id)).c_str());
		  assert(rc == 0);
		}
		if (ok) {
		  control_node.erase(id);
		  std::cout << "OK" << std::endl;
		} else {
		  std::cout << "Error: Node is unavailable" << std::endl;
		}
	  } else {
		std::cout << "Error: Not found" << std::endl;
	  }
	} else if (s == "ping") {
	  int ind = control_node.find(id);
	  if (ind == -1) {
		std::cout << "Error: Not found" << std::endl;
		continue;
	  }
	  auto *token = new node_token_t({ping, id, id});
	  node_token_t reply({fail, id, id});
	  if (my_zmq::send_receive_wait(token, reply, children[ind].second) and reply.action == success) {
		std::cout << "OK: 1" << std::endl;
	  } else {
		std::cout << "OK: 0" << std::endl;
	  }
	}
	else if(s == "exec"){

	}
  }
}
