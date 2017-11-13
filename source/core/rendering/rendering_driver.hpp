#ifndef SU_CORE_RENDERING_DRIVER_HPP
#define SU_CORE_RENDERING_DRIVER_HPP

#include "tile_queue.hpp"
#include "image/typed_image.hpp"
#include <memory>

namespace take { struct View; }

namespace scene {

namespace camera { class Camera; }

class Scene;

}

namespace sampler { class Factory; }

namespace thread { class Pool; }

namespace rendering {

namespace integrator {

namespace surface { class Factory; }
namespace volume { class Factory; }

}

class Camera_worker;

class Driver {

public:

	using Surface_integrator_factory = std::shared_ptr<integrator::surface::Factory>;
	using Volume_integrator_factory  = std::shared_ptr<integrator::volume::Factory>;
	using Sampler_factory			 = std::shared_ptr<sampler::Factory>;

	Driver(const Surface_integrator_factory& surface_integrator_factory,
		   const Volume_integrator_factory& volume_integrator_factory,
		   const Sampler_factory& sampler_factory,
		   scene::Scene& scene,
		   take::View& view,
		   thread::Pool& thread_pool,
		   uint32_t max_sample_size);

	~Driver();

	scene::camera::Camera& camera();

	const scene::Scene& scene() const;

	size_t num_bytes() const;

protected:

	Surface_integrator_factory surface_integrator_factory_;
	Volume_integrator_factory  volume_integrator_factory_;
	Sampler_factory			   sampler_factory_;

	scene::Scene& scene_;
	take::View& view_;
	thread::Pool& thread_pool_;

	Camera_worker* workers_;
	Tile_queue tiles_;

	image::Float4 target_;
};

}

#endif
