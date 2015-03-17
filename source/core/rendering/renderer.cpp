#include "renderer.hpp"
#include "context.hpp"
#include "worker.hpp"
#include "rendering/film/film.hpp"
#include "rendering/integrator/integrator.hpp"
#include "scene/scene.hpp"
#include "base/math/vector.inl"
#include "base/math/random/generator.inl"

namespace rendering {

Renderer::Renderer(std::shared_ptr<Surface_integrator_factory> surface_integrator_factory, std::shared_ptr<sampler::Sampler> sampler) :
	surface_integrator_factory_(surface_integrator_factory), sampler_(sampler) {}

void Renderer::render(const scene::Scene& scene, const Context& context) const {
	auto& film = context.camera->film();

	auto& dimensions = film.dimensions();

/*	sampler::Camera_sample sample;

	for (uint32_t y = 0; y < dimensions.y; ++y) {
		for (uint32_t x = 0; x < dimensions.x; ++x) {
			math::float4 color(float(x) / float(dimensions.x), float(y) / float(dimensions.y), 0.5f, 1.f);
			sample.coordinates = math::float2(static_cast<float>(x), static_cast<float>(y));
			film.add_sample(sample, color.xyz);
		}
	}
	*/

	math::random::Generator rng(0, 1, 2, 3);

	Worker worker(0, rng, *surface_integrator_factory_, *sampler_);

	Rectui tile {math::uint2(0, 0), dimensions};

	worker.render(scene, *context.camera, tile);
}

}
