#include "procedural_sky.hpp"
#include "core/scene/scene.hpp"
#include "core/scene/scene_loader.hpp"
#include "core/scene/entity/dummy.hpp"
#include "core/scene/material/material_provider.hpp"
#include "core/scene/material/light/light_constant.hpp"
#include "core/scene/material/sky/sky_material_clear.hpp"
#include "core/scene/prop/prop.hpp"
#include "base/json/json.hpp"
#include "base/math/vector.inl"
#include "base/math/quaternion.inl"
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
	math::float3x3 sun_rotation(1.f, 0.f, 0.f,
								0.f, 0.f, 1.f,
								0.f, -1.f, 0.f);

	math::float3 ground_albedo(0.3f, 0.3f, 0.3f);
	float turbidity = 1.f;

	extension_value.FindMember("parameters");

	const json::Value::ConstMemberIterator p = extension_value.FindMember("parameters");
	if (extension_value.MemberEnd() != p) {
		const json::Value& parameters_value = p->value;
		for (auto n = parameters_value.MemberBegin(); n != parameters_value.MemberEnd(); ++n) {
			const std::string node_name = n->name.GetString();
			const json::Value& node_value = n->value;

			if ("sun" == node_name) {
				math::float3 angles = json::read_float3(node_value, "rotation");
				sun_rotation = json::create_rotation_matrix(angles);
			} else if ("ground_albedo" == node_name) {
				ground_albedo = json::read_float3(node_value);
			} else if ("turbidity" == node_name) {
				turbidity = json::read_float(node_value);
			}
		}
	}

	scene::entity::Entity* dummy = scene.create_dummy();

	bool visible_in_camera = true;
	bool visible_in_reflection = true;
	bool visible_in_shadow = true;

	// create sky
	{
		auto material = material_provider_->create_clear_sky();

		material->set_sun_direction(math::float3(sun_rotation.z));
		material->set_ground_albedo(ground_albedo);
		material->set_turbidity(turbidity);

		std::vector<std::shared_ptr<scene::material::Material>> materials(1);
		materials[0] = material;

		scene::Prop* prop = scene.create_prop(scene_loader_->canopy(), materials);

		prop->set_visibility(visible_in_camera, visible_in_reflection, visible_in_shadow);
		prop->set_open(false);

		math::transformation transformation {
			math::float3_identity,
			math::float3(1.f, 1.f, 1.f),
			math::create_quaternion_rotation_x(math::degrees_to_radians(90.f))
		};

		prop->set_transformation(transformation);

		scene.create_prop_light(prop, 0);

		dummy->attach(prop);
	}

	// create sun
	{
		auto material = material_provider_->create_light();

		material->set_emission(math::float3(2477907.f, 2133774.f, 1763460.f));

		std::vector<std::shared_ptr<scene::material::Material>> materials(1);
		materials[0] = material;

		scene::Prop* prop = scene.create_prop(scene_loader_->celestial_disk(), materials);

		prop->set_visibility(visible_in_camera, visible_in_reflection, visible_in_shadow);
		prop->set_open(false);

		math::transformation transformation {
			math::float3_identity,
			math::float3(0.26f, 0.26f, 0.26f),
			math::create_quaternion(sun_rotation)
		};

		prop->set_transformation(transformation);

		scene.create_prop_light(prop, 0);

		dummy->attach(prop);
	}

	return dummy;
}

Sky::Sky(std::shared_ptr<scene::material::Material> sky_material) : sky_material_(sky_material) {}

void Sky::set_parameters(const json::Value& /*parameters*/) {}

}}
