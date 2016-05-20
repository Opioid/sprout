#pragma once

#include "core/scene/entity/entity.hpp"
#include "core/scene/entity/entity_extension_provider.hpp"
#include <memory>

namespace scene {

class Loader;

namespace material {

class Material;
class Provider;

}}

namespace procedural { namespace sky {

void init(scene::Loader& loader, scene::material::Provider& material_provider);

class Provider : public scene::entity::Extension_provider {

public:

	Provider();

	void set_scene_loader(scene::Loader& loader);
	void set_material_provider(scene::material::Provider& provider);

	virtual scene::entity::Entity* create_extension(const json::Value& extension_value,
													scene::Scene& scene,
													resource::Manager& manager) final override;

private:

	scene::Loader* scene_loader_;
	scene::material::Provider* material_provider_;
};

class Sky : public scene::entity::Entity {

public:

	Sky(std::shared_ptr<scene::material::Material> sky_material);

	virtual void set_parameters(const json::Value& parameters) final override;

private:

	std::shared_ptr<scene::material::Material> sky_material_;
};

}}
