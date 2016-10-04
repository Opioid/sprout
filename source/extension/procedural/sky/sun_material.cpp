#include "sun_material.hpp"
#include "sky_model.hpp"
#include "core/scene/prop.hpp"
#include "core/scene/scene_renderstate.hpp"
#include "core/scene/scene_worker.hpp"
#include "core/scene/shape/shape_sample.hpp"
#include "core/scene/shape/shape.hpp"
#include "core/scene/material/material_sample.inl"
#include "core/scene/material/material_sample_cache.inl"
#include "core/scene/shape/geometry/hitpoint.inl"
#include "base/math/vector.inl"

namespace procedural { namespace sky {

Sun_material::Sun_material(
		scene::material::Sample_cache<scene::material::light::Sample>& cache,
		Model& model) : Material(cache, model) {}

const scene::material::Sample& Sun_material::sample(float3_p wo, const scene::Renderstate& rs,
													const scene::Worker& worker,
													Sampler_filter /*filter*/) {
	auto& sample = cache_.get(worker.id());

	sample.set_basis(rs.geo_n, wo);

	sample.layer_.set_basis(rs.t, rs.b, rs.n);

	sample.layer_.set(model_.evaluate_sky_and_sun(-wo));

	return sample;
}

float3 Sun_material::sample_radiance(float3_p wi, float2 /*uv*/,
									 float /*area*/, float /*time*/,
									 const scene::Worker& /*worker*/,
									 Sampler_filter /*filter*/) const {
	return model_.evaluate_sky_and_sun(wi);
}

float3 Sun_material::average_radiance(float /*area*/) const {
	return model_.evaluate_sky_and_sun(-model_.sun_direction());
}

void Sun_material::prepare_sampling(const scene::shape::Shape& /*shape*/, uint32_t /*part*/,
									const Transformation& /*transformation*/,
									float /*area*/, bool /*importance_sampling*/,
									thread::Pool& /*pool*/) {
	model_.init();
}

size_t Sun_material::num_bytes() const {
	return sizeof(*this);
}

}}
