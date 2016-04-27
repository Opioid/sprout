#pragma once

#include "rendering_driver.hpp"

namespace scene { namespace camera { class Camera; } }

namespace exporting { class Sink; }

namespace progress { class Sink; }

namespace rendering {

class Driver_finalframe : public Driver {

public:

	Driver_finalframe(Surface_integrator_factory surface_integrator_factory,
					  Volume_integrator_factory volume_integrator_factory,
					  std::shared_ptr<sampler::Sampler> sampler,
					  scene::Scene& scene, const take::View& view,
					  thread::Pool& thread_pool);

	void render(exporting::Sink& exporter, progress::Sink& progressor);

private:

	void render_subframe(float normalized_tick_offset,
						 float normalized_tick_slice,
						 float normalized_frame_slice,
						 progress::Sink& progressor);

	static uint32_t calculate_progress_range(const scene::Scene& scene,
											 const scene::camera::Camera& camera,
											 uint32_t num_tiles,
											 uint32_t num_samples_per_iteration);

	uint32_t current_sample_;
};

}
