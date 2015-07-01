#pragma once

#include "base/math/vector.hpp"
#include <vector>
#include <memory>

namespace scene {

namespace camera {

class Camera;

}

class Scene;

}

namespace sampler {

class Sampler;

}

namespace thread {

class Pool;

}

namespace exporting {

class Sink;

}

namespace progress {

class Sink;

}

namespace rendering {

class Tile_queue;
class Worker;
class Surface_integrator_factory;
struct Context;

class Renderer {
public:

	Renderer(std::shared_ptr<Surface_integrator_factory> surface_integrator_factory,
			 std::shared_ptr<sampler::Sampler> sampler);

	void render(scene::Scene& scene, const Context& context, thread::Pool& pool, exporting::Sink& exporter, progress::Sink& progressor);

private:

	void render_subframe(const scene::camera::Camera& camera,
						 float normalized_tick_offset, float normalized_tick_slice,
						 float normalized_frame_offset, float normalized_frame_slice,
						 Tile_queue& tiles, std::vector<Worker>& workers, thread::Pool& pool, progress::Sink& progressor);

	bool advance_current_pixel(const math::uint2& dimensions);

	std::shared_ptr<Surface_integrator_factory> surface_integrator_factory_;
	std::shared_ptr<sampler::Sampler> sampler_;

	math::uint2 tile_dimensions_;
	math::uint2 current_pixel_;
};

}
