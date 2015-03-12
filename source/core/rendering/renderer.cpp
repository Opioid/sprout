#include "renderer.hpp"
#include "context.hpp"
#include "rendering/film/film.hpp"
#include "rendering/sampler/camera_sample.hpp"
#include "scene/scene.hpp"
#include "base/math/vector.inl"

namespace rendering {

void Renderer::render(const scene::Scene& scene, const Context& context) const {
	auto& film = context.camera->film();

	auto& dimensions = film.dimensions();

	sampler::Camera_sample sample;

	for (uint32_t y = 0; y < dimensions.y; ++y) {
		for (uint32_t x = 0; x < dimensions.x; ++x) {
			math::float4 color(float(x) / float(dimensions.x), float(y) / float(dimensions.y), 0.5f, 1.f);
			sample.coordinates = math::float2(static_cast<float>(x), static_cast<float>(y));
			film.add_sample(sample, color.xyz);
		}
	}
}

}
