#pragma once

#include "rendering_camera_worker.hpp"
#include "tile_queue.hpp"
#include "image/typed_image.hpp"
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

namespace integrator {

namespace surface { class Integrator_factory; }
namespace volume { class Integrator_factory; }

}

class Driver {
public:

	using Surface_integrator_factory = std::shared_ptr<integrator::surface::Integrator_factory>;
	using Volume_integrator_factory  = std::shared_ptr<integrator::volume::Integrator_factory>;

	Driver(Surface_integrator_factory surface_integrator_factory,
		   Volume_integrator_factory volume_integrator_factory,
		   std::shared_ptr<sampler::Sampler> sampler,
		   scene::Scene& scene, const take::View& view,
		   thread::Pool& thread_pool);

protected:

	Surface_integrator_factory surface_integrator_factory_;
	Volume_integrator_factory volume_integrator_factory_;
	std::shared_ptr<sampler::Sampler> sampler_;

	scene::Scene& scene_;
	const take::View& view_;
	thread::Pool& thread_pool_;

	std::vector<Camera_worker> workers_;
	Tile_queue tiles_;

	image::Image_float_4 target_;
};

}
