#pragma once

#include "id_manager.hpp"
#include <string>

namespace resource {

template<typename T>
class Identifiable {

public:

	Identifiable(const std::string& name) : name_(name) {
		s_id = id_manager.id(name);
	}

	const std::string& name() const {
		return name_;
	}

	static uint32_t id() {
		return s_id;
	}

private:

	std::string name_;

	static uint32_t s_id;
};

template<typename T> uint32_t Identifiable<T>::s_id = 0xFFFFFF;

}
