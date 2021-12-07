#ifndef INC_6_8_LAB__TOPOLOGY_H_
#define INC_6_8_LAB__TOPOLOGY_H_

#include <iostream>
#include <list>
#include <map>

using node_id_type = long long;

template<typename T>
class topology_t {
 private:
  using list_type = std::list<std::list<T>>;
  using iterator = typename std::list<T>::iterator;
  using list_iterator = typename list_type::iterator;

  list_type container;
  size_t container_size;

 public:
  topology_t() : container(), container_size(0){};
  ~topology_t() = default;

  void insert(const T &elem) {
	std::list<T> new_list;
	new_list.emplace_back(elem);
	++container_size;
	container.emplace_back(new_list);
  }

  bool insert(const T &parent, const T &elem) {
	for (list_iterator external_it = container.begin(); external_it != container.end(); ++external_it) {
	  for (iterator internal_it = external_it->begin(); internal_it != external_it->end(); ++internal_it) {
		if (*internal_it == parent) {
		  external_it->insert(++internal_it, elem);
		  ++container_size;
		  return true;
		}
	  }
	}
	return false;
  }

  bool erase(const T &elem) {
	for (list_iterator external_it = container.begin(); external_it != container.end(); ++external_it) {
	  for (iterator internal_it = external_it->begin(); internal_it != external_it->end(); ++internal_it) {
		if (*internal_it == elem) {
		  if (external_it->size() > 1) {
			external_it->erase(internal_it);
		  } else {
			container.erase(external_it);
		  }
		  --container_size;
		  return true;
		}
	  }
	}
	return false;
  }

  size_t size() {
	return container_size;
  }
  int find(const T &elem) {// in which list exists (or not) element with id $id
	int ind = 0;
	for (auto &external : container) {
	  for (auto &internal : external) {
		if (internal == elem) {
		  return ind;
		}
	  }
	  ++ind;
	}
	return -1;
  }
  template<typename S>
  friend std::ostream &operator<<(std::ostream &os, const topology_t<S> &topology) {
	for (auto &external : topology.container) {
	  os << "{";
	  for (auto &internal : external) {
		os << internal << " ";
	  }
	  os << "}" << std::endl;
	}
	return os;
  }
};

#endif//INC_6_8_LAB__TOPOLOGY_H_
