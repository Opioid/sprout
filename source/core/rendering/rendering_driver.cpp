#include "rendering_driver.hpp"
#include "image/typed_image.inl"
#include "sampler/sampler.hpp"
#include "scene/scene.hpp"
#include "scene/camera/camera.hpp"
#include "rendering/sensor/sensor.hpp"
#include "take/take_view.hpp"
#include "base/math/vector.inl"
#include "base/math/random/generator.inl"
#include "base/thread/thread_pool.hpp"

namespace rendering {

Driver::Driver(Surface_integrator_factory surface_integrator_factory,
			   Volume_integrator_factory volume_integrator_factory,
			   std::shared_ptr<sampler::Factory> sampler_factory,
			   scene::Scene& scene,
			   take::View& view,
			   thread::Pool& thread_pool) :
	surface_integrator_factory_(surface_integrator_factory),
	volume_integrator_factory_(volume_integrator_factory),
	sampler_factory_(sampler_factory),
	scene_(scene), view_(view), thread_pool_(thread_pool),
	workers_(thread_pool.num_threads()),
	tiles_(view.camera->resolution(), int2(32, 32),
		   view.camera->sensor().filter_radius_int()),
	target_(image::Image::Description(image::Image::Type::Float_4,
									  view.camera->sensor_dimensions())) {
	for (uint32_t i = 0, len = thread_pool.num_threads(); i < len; ++i) {
		math::random::Generator rng(i + 0, i + 1, i + 2, i + 3);
		workers_[i].init(i, scene, rng, *surface_integrator_factory,
						 *volume_integrator_factory, *sampler_factory_);
	}
}

Driver::~Driver() {}

scene::camera::Camera& Driver::camera() {
	return *view_.camera;
}

const scene::Scene& Driver::scene() const {
	return scene_;
}

}
