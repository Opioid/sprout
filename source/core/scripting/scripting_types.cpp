#include "scripting_types.hpp"
#include "scripting_engine.hpp"
#include "dukglue/dukglue.hpp"

#include "scene/scene.hpp"
#include "scene/entity/entity.hpp"

namespace scripting {

void register_types(Engine& engine) {
	const auto context = engine.context();

	using namespace scene;

	const uint32_t primitive_id = 0xFFFFFFFF;

	// Scene
	const uint32_t scene_id = class_id(Type::Scene);
	const uint32_t entity_id = class_id(Type::Entity);

	dukglue_register_method(context, scene_id, &Scene::current_tick, primitive_id, "currentTick");
	dukglue_register_method(context, scene_id, &Scene::simulation_time, primitive_id, "simulationTime");

	dukglue_register_method<Scene, entity::Entity*, size_t>(context, scene_id, &Scene::entity, entity_id, "entityByIndex");
	dukglue_register_method<Scene, entity::Entity*, std::string_view>(context, scene_id, &Scene::entity, entity_id, "entityByName");

	// Entity
	using namespace scene::entity;

	dukglue_register_method(context, entity_id, &Entity::visible_in_camera, primitive_id, "visibleInCamera");

//	dukglue_set_base_class(context, entity_id, class_id(Type::Prop));

}

uint32_t class_id(Type type) {
	return static_cast<uint32_t>(type);
}

}
