#include "procedural_sky.hpp"
#include "core/scene/scene.hpp"
#include "core/scene/scene_loader.hpp"
#include "core/scene/material/material_provider.hpp"
#include "core/scene/material/sky/sky_material_clear.hpp"
#include "core/scene/prop/prop.hpp"
#include <iostream>

namespace procedural { namespace sky {

Provider provider;

void init(scene::Loader& loader, scene::material::Provider& material_provider) {
	provider.set_scene_loader(loader);
	provider.set_material_provider(material_provider);

	loader.register_extension_provider("Sky", &provider);
}

Provider::Provider() : material_provider_(nullptr) {}

void Provider::set_scene_loader(scene::Loader& loader) {
	scene_loader_ = &loader;
}

void Provider::set_material_provider(scene::material::Provider& provider) {
	material_provider_ = &provider;
}

scene::entity::Entity* Provider::create_extension(const json::Value& extension_value,
												  scene::Scene& scene,
												  resource::Manager& /*manager*/) {
	auto material = material_provider_->create_clear_sky();

	math::float3 sun_direction;
	math::float3 ground_albedo(0.3f, 0.3f, 0.3f);
	float turbidity = 1.f;

	sun_direction = math::normalized(math::float3(1.f, 1.f, 1.f));

	material->set_sun_direction(sun_direction);
	material->set_ground_albedo(ground_albedo);
	material->set_turbidity(turbidity);

	scene.add_material(material);

	std::vector<std::shared_ptr<scene::material::Material>> materials(1);
	materials[0] = material;

	scene::Prop* prop = scene.create_prop(scene_loader_->canopy(), materials);

	bool visible_in_camera = true;
	bool visible_in_reflection = true;
	bool visible_in_shadow = true;

	prop->set_visibility(visible_in_camera, visible_in_reflection, visible_in_shadow);
	prop->set_open(false);

	scene.create_prop_light(prop, 0);

	return prop;
}

}}
