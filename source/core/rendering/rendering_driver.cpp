#include "rendering_driver.hpp"
#include "rendering_camera_worker.hpp"
#include "image/typed_image.inl"
#include "sampler/sampler.hpp"
#include "scene/scene.hpp"
#include "scene/camera/camera.hpp"
#include "rendering/sensor/sensor.hpp"
#include "take/take.hpp"
#include "take/take_view.hpp"
#include "base/memory/align.hpp"
#include "base/math/vector4.inl"
#include "base/random/generator.inl"
#include "base/thread/thread_pool.hpp"

namespace rendering {

Driver::Driver(take::Take& take, scene::Scene& scene,
			   thread::Pool& thread_pool, uint32_t max_sample_size) :
	scene_(scene), view_(take.view), thread_pool_(thread_pool),
	workers_(memory::construct_aligned<Camera_worker>(thread_pool.num_threads())),
	tiles_(take.view.camera->resolution(), int2(32, 32),
		   take.view.camera->sensor().filter_radius_int()),
	target_(image::Image::Description(image::Image::Type::Float4,
									  take.view.camera->sensor_dimensions())) {
	for (uint32_t i = 0, len = thread_pool.num_threads(); i < len; ++i) {
		workers_[i].init(i, take.settings, scene, max_sample_size,
						 *take.surface_integrator_factory,
						 *take.volume_integrator_factory,
						 *take.sampler_factory);
	}
}

Driver::~Driver() {
	memory::destroy_aligned(workers_, thread_pool_.num_threads());
}

scene::camera::Camera& Driver::camera() {
	return *view_.camera;
}

const scene::Scene& Driver::scene() const {
	return scene_;
}

size_t Driver::num_bytes() const {
	// Every worker must have exactly the same size, so we only need to query a single one
	size_t worker_num_bytes = thread_pool_.num_threads() * workers_[0].num_bytes();

	return worker_num_bytes + target_.num_bytes();
}

}
