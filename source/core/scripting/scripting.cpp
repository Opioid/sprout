#include "scripting.hpp"
#include "scripting_engine.hpp"
#include "dukglue/dukglue.h"

#include "scene/scene.hpp"

namespace scripting {

void register_types(Engine& engine) {
	auto context = engine.context();

	dukglue_register_constructor<scene::Scene>(context, "Scene");
	dukglue_register_method(context, &scene::Scene::current_tick, "current_tick");
}

}
