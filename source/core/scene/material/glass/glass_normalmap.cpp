#include "glass_normalmap.hpp"
#include "scene/material/material_sample_cache.inl"
#include "scene/shape/geometry/differential.hpp"
#include "image/texture/texture_2d.inl"
#include "image/texture/sampler/sampler_2d.hpp"
#include <algorithm>

// https://seblagarde.wordpress.com/2011/08/17/feeding-a-physical-based-lighting-mode/

namespace scene { namespace material { namespace glass {

Normalmap::Normalmap(Sample_cache<Sample>& cache, std::shared_ptr<image::Image> mask,
					 const math::float3& color, float attenuation_distance, float ior, std::shared_ptr<image::Image> normal) :
	Glass(cache, mask), color_(color),
	attenuation_(1.f / (color.x * attenuation_distance), 1.f / (color.y * attenuation_distance), 1.f / (color.z * attenuation_distance)),
	ior_(std::max(ior, 1.0001f)),
	normal_(normal) {
	float n = ior - 1.f;
	float d = ior + 1.f;
	f0_ = (n * n) / (d * d);
}

const Sample& Normalmap::sample(const shape::Differential& dg, const math::float3& wo,
								const image::sampler::Sampler_2D& sampler, uint32_t worker_id) {
	auto& sample = cache_.get(worker_id);

	math::float3 nm = sampler.sample3(normal_, dg.uv);
	math::float3 n = math::normalized(dg.tangent_to_world(nm));

//	math::float3 t;
//	math::float3 b;
//	math::coordinate_system(n, t, b);

	sample.set_basis(dg.t, dg.b, n, dg.geo_n, wo);

	sample.set(color_, attenuation_, ior_, f0_);

	return sample;
}

}}}
