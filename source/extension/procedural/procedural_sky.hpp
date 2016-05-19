#pragma once

#include "core/scene/entity/entity_extension_provider.hpp"

namespace scene { class Loader; }

namespace procedural { namespace sky {

void init(scene::Loader& loader);

class Provider : public scene::entity::Extension_provider {

public:

	virtual scene::entity::Entity* create_extension(const json::Value& extension_value,
													scene::Scene& scene,
													resource::Manager& manager) final override;
};

}}
