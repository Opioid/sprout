#include "sky_provider.hpp"
#include "sky.hpp"
#include "sky_material.hpp"
#include "sun_material.hpp"
#include "core/resource/resource_manager.inl"
#include "core/scene/scene.hpp"
#include "core/scene/scene_loader.hpp"
#include "core/scene/material/material_provider.hpp"
#include "core/scene/material/light/light_constant.hpp"
#include "core/scene/prop/prop.hpp"
#include "base/json/json.hpp"
#include "base/math/matrix4x4.inl"
#include "base/math/vector3.inl"
#include "base/math/quaternion.inl"

namespace procedural::sky {

using namespace scene;

Provider provider;

void init(scene::Loader& loader, material::Provider& material_provider) {
	provider.set_scene_loader(loader);
	provider.set_material_provider(material_provider);

	loader.register_extension_provider("Sky", &provider);
}

void Provider::set_scene_loader(Loader& loader) {
	scene_loader_ = &loader;
}

void Provider::set_material_provider(material::Provider& material_provider) {
	material_provider_ = &material_provider;
}

entity::Entity* Provider::create_extension(const json::Value& extension_value,
										   Scene& scene, resource::Manager& manager) {
	Sky* sky = new Sky;

	const bool bake = true;

	std::shared_ptr<Material> sky_material;

	if (bake) {
		sky_material = std::make_shared<Sky_baked_material>(sky->model());
	} else {
		sky_material = std::make_shared<Sky_material>(sky->model());
	}

	manager.store<material::Material>("proc:sky", sky_material);

	auto sun_material = std::make_shared<Sun_material>(sky->model());

	manager.store<material::Material>("proc:sun", sun_material);

	Materials materials(1);

	materials[0] = sky_material;
	prop::Prop* sky_prop = scene.create_prop(scene_loader_->canopy(), materials);

	materials[0] = sun_material;
	prop::Prop* sun_prop = scene.create_prop(scene_loader_->celestial_disk(), materials);

	sky->init(sky_prop, sun_prop);
	sky->set_propagate_visibility(true);

	const json::Value::ConstMemberIterator p = extension_value.FindMember("parameters");
	if (extension_value.MemberEnd() != p) {
		sky->set_parameters(p->value);
	}

	const bool visible_in_camera = true;
	const bool visible_in_reflection = true;
	const bool visible_in_shadow = true;

	sky_prop->set_visibility(visible_in_camera, visible_in_reflection, visible_in_shadow);
	sky_prop->set_open(false);

	if (bake) {
		scene.create_prop_image_light(sky_prop, 0);
	} else {
		scene.create_prop_light(sky_prop, 0);
	}

	sun_prop->set_visibility(visible_in_camera, visible_in_reflection, visible_in_shadow);
	sun_prop->set_open(false);
	scene.create_prop_light(sun_prop, 0);

	return sky;
}

}
