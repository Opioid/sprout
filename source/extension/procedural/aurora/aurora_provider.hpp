#ifndef SU_EXTENSION_PROCEDURAL_AURORA_PROVIDER_HPP
#define SU_EXTENSION_PROCEDURAL_AURORA_PROVIDER_HPP

#include "core/image/typed_image_fwd.hpp"
#include "core/scene/entity/entity_extension_provider.hpp"
#include "base/math/vector3.hpp"

namespace rnd { class Generator; }

namespace scene { class Loader; }

namespace thread { class Pool; }

namespace procedural::aurora {

class Volume_rasterizer;

void init(scene::Loader& loader);

class Provider : public scene::entity::Extension_provider {

public:

	void set_scene_loader(scene::Loader& loader);

	virtual scene::entity::Entity* create_extension(const json::Value& extension_value,
													scene::Scene& scene,
													resource::Manager& manager) override final;

private:

	void render(image::Byte3& target, thread::Pool& thread_pool);

	void simulate_particle(const float3& start, float peak_height, rnd::Generator& rng,
						   Volume_rasterizer& renderer) const;

	float3 world_to_grid(f_float3 position) const;

	scene::Loader* scene_loader_ = nullptr;

	float voxel_ratio_;

	float3 aurora_volume_;

	static constexpr float ground_to_bottom_ = 100000.f;
	static constexpr float aurora_height_    = 200000.f;
};

}

#endif
