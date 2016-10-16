#pragma once

#include "rendering_camera_worker.hpp"
#include "tile_queue.hpp"
#include "image/typed_image.hpp"
#include <memory>
#include <vector>

namespace take { struct View; }

namespace scene { class Scene; }

namespace sampler { class Factory; }

namespace thread { class Pool; }

namespace rendering {

namespace integrator {

namespace surface { class Factory; }
namespace volume { class Factory; }

}

class Driver {

public:

	using Surface_integrator_factory = std::shared_ptr<integrator::surface::Factory>;
	using Volume_integrator_factory  = std::shared_ptr<integrator::volume::Factory>;

	Driver(Surface_integrator_factory surface_integrator_factory,
		   Volume_integrator_factory volume_integrator_factory,
		   std::shared_ptr<sampler::Factory> sampler_factory,
		   scene::Scene& scene,
		   take::View& view,
		   thread::Pool& thread_pool);

	~Driver();

	scene::camera::Camera& camera();

	const scene::Scene& scene() const;

	size_t num_bytes() const;

protected:

	Surface_integrator_factory surface_integrator_factory_;
	Volume_integrator_factory volume_integrator_factory_;
	std::shared_ptr<sampler::Factory> sampler_factory_;

	scene::Scene& scene_;
	take::View& view_;
	thread::Pool& thread_pool_;

	std::vector<Camera_worker> workers_;
	Tile_queue tiles_;

	image::Image_float_4 target_;
};

}
