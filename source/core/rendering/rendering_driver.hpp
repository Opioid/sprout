#ifndef SU_CORE_RENDERING_DRIVER_HPP
#define SU_CORE_RENDERING_DRIVER_HPP

#include "tile_queue.hpp"
#include "image/typed_image.hpp"
#include <memory>

namespace take { struct Take; struct View; }

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

	Driver(take::Take& take, scene::Scene& scene,
		   thread::Pool& thread_pool, uint32_t max_sample_size);

	~Driver();

	scene::camera::Camera& camera();

	const scene::Scene& scene() const;

	size_t num_bytes() const;

protected:

	scene::Scene& scene_;
	take::View& view_;
	thread::Pool& thread_pool_;

	Camera_worker* workers_;
	Tile_queue tiles_;

	image::Float4 target_;
};

}

#endif
