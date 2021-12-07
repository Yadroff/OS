#include <unistd.h>
#include <vector>
#include <zmq.hpp>

#include "my_zmq.h"
#include "topology.h"

using node_id_type = long long;

int main() {
  int rc;
  bool ok;
  topology_t<node_id_type> control_node;
  std::vector<std::pair<void *, void *>> children;// [context, socket]
  std::string s;
  node_id_type id;
  while (std::cin >> s >> id) {
	if (s == "create") {
	  node_id_type parent_id;
	  std::cin >> parent_id;
	  int ind;
	  if (parent_id == -1) {
		void *new_context = nullptr;
		void *new_socket = nullptr;
		my_zmq::init_pair_socket(new_context, new_socket);
		rc = zmq_bind(new_socket, ("tcp://*:" + std::to_string(PORT_BASE + id)).c_str());
		assert(rc == 0);

		int fork_id = fork();
		if (fork_id == 0) {
		  rc = execl(NODE_EXECUTABLE_NAME, NODE_EXECUTABLE_NAME, std::to_string(id).c_str(), nullptr);
		  assert(rc != -1);
		  return 0;
		} else {
		  auto *token = new node_token_t({ping, id, id});
		  node_token_t reply({fail, id, id});
		  if (my_zmq::send_receive_wait(token, reply, new_socket) and reply.action == success) {
			children.emplace_back(std::make_pair(new_context, new_socket));
			control_node.insert(id);
		  } else {
			rc = zmq_close(new_socket);
			assert(rc == 0);
			rc = zmq_ctx_term(new_context);
			assert(rc == 0);
		  }
		}
	  } else if ((ind = control_node.find(parent_id)) == -1) {
		std::cout << "Error: Not found" << std::endl;
		continue;
	  } else {
		if (control_node.find(id) != -1) {
		  std::cout << "Error: Already exists" << std::endl;
		  continue;
		}
		auto *token = new node_token_t({create, parent_id, id});
		node_token_t reply({fail, id, id});
		if (my_zmq::send_receive_wait(token, reply, children[ind].second) and reply.action == success) {
		  control_node.insert(parent_id, id);
		} else {
		  std::cout << "Error: Parent is unavailable" << std::endl;
		}
	  }
	} else if (s == "remove") {
	  int ind = control_node.find(id);
	  if (ind != -1) {
		auto *token = new node_token_t({destroy, id, id});
		node_token_t reply({fail, id, id});
		ok = my_zmq::send_receive_wait(token, reply, children[ind].second);
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
	} else if (s == "exec") {
	  ok = true;
	  std::string key;
	  char c;
	  int val = -1;
	  bool add = false;
	  std::cin >> key;
	  if ((c = getchar()) == ' '){
		add = true;
		std::cin >> val;
	  }
	  int ind = control_node.find(id);
	  if (ind == -1) {
		std::cout << "Error: Not found" << std::endl;
		continue;
	  }
	  key += SENTINEL;
	  if (add) {
		for (auto i: key) {
		  auto *token = new node_token_t({exec_add, i, id});
		  node_token_t reply({fail, id, id});
		  if (!my_zmq::send_receive_wait(token, reply, children[ind].second) or reply.action != success) {
			std::cout << "Fail: " << i << std::endl;
			ok = false;
			break;
		  }
		}
		auto *token = new node_token_t({exec_add, val, id});
		node_token_t reply({fail, id, id});
		if (!my_zmq::send_receive_wait(token, reply, children[ind].second) or reply.action != success) {
		  std::cout << "Fail: " << val << std::endl;
		  ok = false;
		}
	  } else {
		for (auto i: key) {
		  auto *token = new node_token_t({exec_check, i, id});
		  node_token_t reply({fail, i, id});
		  if (!my_zmq::send_receive_wait(token, reply, children[ind].second) or reply.action != success) {
			ok = false;
			std::cout << "Fail: " << i << std::endl;
			break;
		  }
		}
	  }
	  if (!ok){
		std::cout << "Error: Node is unavailable" << std::endl;
	  }
	}
  }

  for (auto [context, socket]: children) {
	rc = zmq_close(socket);
	assert(rc == 0);
	rc = zmq_ctx_term(context);
	assert(rc == 0);
  }
}