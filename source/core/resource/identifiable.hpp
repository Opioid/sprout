#pragma once

#include "id_manager.hpp"
#include <string>

namespace resource {

template<typename T>
class Identifiable {

public:

	Identifiable(const std::string& name) {
		type_id_ = id_manager.id(name);
	}

	static uint32_t id() {
		return type_id_;
	}

	static const std::string& name() {
		return id_manager.name(type_id_);
	}

private:

	static uint32_t type_id_;
};

template<typename T> uint32_t Identifiable<T>::type_id_ = 0xFFFFFF;

}
