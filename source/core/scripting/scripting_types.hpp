#ifndef SU_CORE_SCRIPTING_TYPES_HPP
#define SU_CORE_SCRIPTING_TYPES_HPP

#include <cstdint>

namespace scripting {

enum class Type {
	Entity,
	Math,
	Prop,
	Scene
};

class Engine;

void register_types(Engine& engine);

uint32_t class_id(Type type);

}

#endif
