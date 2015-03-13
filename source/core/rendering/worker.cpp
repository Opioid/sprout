#include "worker.hpp"
#include "rendering/sampler/camera_sample.hpp"
#include "rendering/film/film.hpp"
#include "scene/camera/camera.hpp"
#include "base/math/vector.inl"

namespace rendering {

void Worker::render(const scene::Scene& scene, const camera::Camera& camera, const Rectui& tile) const {
	auto& film = camera.film();

	auto& dimensions = film.dimensions();

	sampler::Camera_sample sample;

	for (uint32_t y = tile.start.y; y < tile.end.y; ++y) {
		for (uint32_t x = tile.start.x; x < tile.end.x; ++x) {
			math::float4 color(float(x) / float(dimensions.x), float(y) / float(dimensions.y), 0.5f, 1.f);
			sample.coordinates = math::float2(static_cast<float>(x), static_cast<float>(y));
			film.add_sample(sample, color.xyz);
		}
	}
}

}
