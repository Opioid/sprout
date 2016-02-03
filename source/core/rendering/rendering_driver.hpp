#pragma once

#include "base/math/vector.hpp"
#include <memory>
#include <vector>

namespace take { struct View; }

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

namespace integrator {

namespace surface { class Integrator_factory; }
namespace volume { class Integrator_factory; }

}

class Driver {
public:

	Driver(std::shared_ptr<integrator::surface::Integrator_factory> surface_integrator_factory,
		   std::shared_ptr<integrator::volume::Integrator_factory> volume_integrator_factory,
		   std::shared_ptr<sampler::Sampler> sampler);

	void render(scene::Scene& scene, const take::View& view, thread::Pool& thread_pool,
				exporting::Sink& exporter, progress::Sink& progressor);

private:

	void render_subframe(scene::camera::Camera& camera,
						 float absolute_time, float tick_slice,
						 float normalized_tick_offset, float normalized_tick_slice, float normalized_frame_slice,
						 Tile_queue& tiles, std::vector<Camera_worker>& workers, thread::Pool& pool,
						 progress::Sink& progressor);

	uint32_t calculate_progress_range(const scene::Scene& scene,
									  const scene::camera::Camera& camera, uint32_t num_tiles) const;

	std::shared_ptr<integrator::surface::Integrator_factory> surface_integrator_factory_;
	std::shared_ptr<integrator::volume::Integrator_factory> volume_integrator_factory_;
	std::shared_ptr<sampler::Sampler> sampler_;

	math::int2 tile_dimensions_;
	uint32_t   current_sample_;
};

}
