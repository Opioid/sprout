#include "scripting_types.hpp"
#include "scripting_types.inl"
#include "scripting_engine.hpp"
#include "scripting_math.hpp"
#include "dukglue/dukglue.hpp"

#include "scene/scene.hpp"
#include "scene/entity/entity.hpp"

namespace scripting {

Math math;

void register_types(Engine& engine) {
	const auto context = engine.context();

	using namespace scene;

	// Scene
	const uint32_t scene_id = class_id(Type::Scene);
	const uint32_t entity_id = class_id(Type::Entity);

	dukglue_register_method(context, scene_id, &Scene::current_tick, "currentTick");
	dukglue_register_method(context, scene_id, &Scene::simulation_time, "simulationTime");

	dukglue_register_method<Scene, entity::Entity*, size_t>(
				context, scene_id, &Scene::entity, entity_id, "entityByIndex");
	dukglue_register_method<Scene, entity::Entity*, std::string_view>(
				context, scene_id, &Scene::entity, entity_id, "entityByName");

	// Entity
	using namespace scene::entity;

	dukglue_register_method(context, entity_id, &Entity::visible_in_camera, "visibleInCamera");

//	dukglue_set_base_class(context, entity_id, class_id(Type::Prop));


	// Math
	const uint32_t math_id = class_id(Type::Math);
	dukglue_register_method(context, math_id, &Math::sin, "sin");
	dukglue_register_method(context, math_id, &Math::cos, "cos");

	register_global(engine, Type::Math, &math, "Math");
}

uint32_t class_id(Type type) {
	return static_cast<uint32_t>(type);
}

}
