#ifndef SU_CORE_SCRIPTING_SCRIPTING_TYPES_INL
#define SU_CORE_SCRIPTING_SCRIPTING_TYPES_INL

#include "scripting_types.hpp"
#include "scripting_engine.hpp"
#include "dukglue/dukglue.hpp"

namespace scripting {

template<typename T>
void register_global(const Engine& engine, Type type, const T& object, std::string_view name) {
	dukglue_register_global(engine.context(), class_id(type), object, name.data());
}

}

#endif
