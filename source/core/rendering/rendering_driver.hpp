#pragma once

#include "base/math/vector.hpp"
#include <vector>
#include <memory>

namespace scene {

namespace camera { class Camera; }

class Scene;

}

namespace sampler { class Sampler; }

namespace thread { class Pool; }

namespace exporting { class Sink; }

namespace progress { class Sink; }

namespace rendering {

class Tile_queue;
class Camera_worker;
class Surface_integrator_factory;
struct Context;

class Driver {
public:

	Driver(std::shared_ptr<Surface_integrator_factory> surface_integrator_factory,
		   std::shared_ptr<sampler::Sampler> sampler);

	void render(scene::Scene& scene, const Context& context, thread::Pool& pool,
				exporting::Sink& exporter, progress::Sink& progressor);

private:

	void render_subframe(scene::camera::Camera& camera,
						 float normalized_tick_offset, float normalized_tick_slice, float normalized_frame_slice,
						 Tile_queue& tiles, std::vector<Camera_worker>& workers, thread::Pool& pool,
						 progress::Sink& progressor);

	bool advance_current_pixel(math::uint2 dimensions);

	size_t calculate_progress_range(const scene::Scene& scene,
									const scene::camera::Camera& camera, size_t num_tiles) const;

	std::shared_ptr<Surface_integrator_factory> surface_integrator_factory_;
	std::shared_ptr<sampler::Sampler> sampler_;

	math::uint2 tile_dimensions_;
	math::uint2 current_pixel_;
	uint32_t    current_sample_;
};

}
