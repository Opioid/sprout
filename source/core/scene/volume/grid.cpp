#include "grid.hpp"
#include "image/texture/texture.hpp"
#include "image/texture/texture_adapter.inl"
#include "image/texture/sampler/address_mode.hpp"
#include "image/texture/sampler/sampler_nearest_3d.inl"
#include "image/texture/sampler/sampler_linear_3d.inl"
#include "scene/scene_worker.hpp"
#include "base/json/json.hpp"
#include "base/math/aabb.inl"
#include "base/math/ray.inl"
#include "base/math/vector3.inl"
#include "base/random/generator.inl"

namespace scene::volume {

Grid::Grid(const Texture_ptr& grid) : grid_(grid) {}

float Grid::density(const Transformation& /*transformation*/, const float3& p,
					Sampler_filter filter, const Worker& worker) const {
	// p is in object space already

	float3 p_g = 0.5f * (float3(1.f) + p);
	p_g[1] = 1.f - p_g[1];

	const auto& sampler = worker.sampler_3D(static_cast<uint32_t>(Sampler_filter::Linear), filter);

	return grid_.sample_1(sampler, p_g);
}

void Grid::set_parameter(const std::string& /*name*/, const json::Value& /*value*/) {}

Emission_grid::Emission_grid(const Texture_ptr& grid) : grid_(grid) {}

float3 Emission_grid::emission(const Transformation& transformation, const math::Ray& ray,
							   float step_size, rnd::Generator& rng,
							   Sampler_filter filter, const Worker& worker) const {
	const math::Ray rn = ray.normalized();

	float min_t = rn.min_t + rng.random_float() * step_size;

	float3 emission(0.f);

	const float3 rp_o = math::transform_point(rn.origin, transformation.world_to_object);
	const float3 rd_o = math::transform_vector(rn.direction, transformation.world_to_object);

	for (; min_t < rn.max_t; min_t += step_size) {
		const float3 p_o = rp_o + min_t * rd_o; // r_o.point(min_t);
		emission += Emission_grid::emission(p_o, filter, worker);
	}

	return step_size * emission;
}

float3 Emission_grid::optical_depth(const Transformation& /*transformation*/,
									const math::Ray& /*ray*/, float /*step_size*/,
									rnd::Generator& /*rng*/, Sampler_filter /*filter*/,
									const Worker& /*worker*/) const {
	return float3(0.f);
}


float3 Emission_grid::scattering(const Transformation& /*transformation*/, const float3& /*p*/,
								 Sampler_filter /*filter*/, const Worker& /*worker*/) const {
	return float3(0.f);
}

float3 Emission_grid::emission(const float3& p, Sampler_filter filter, const Worker& worker) const {
	// p is in object space already

	float3 p_g = 0.5f * (float3(1.f) + p);
	p_g[1] = 1.f - p_g[1];

	const auto& sampler = worker.sampler_3D(static_cast<uint32_t>(Sampler_filter::Linear), filter);

	return 0.00001f * grid_.sample_3(sampler, p_g);

//	return grid_.sample_3(sampler, p_g);
}

void Emission_grid::set_parameter(const std::string& /*name*/, const json::Value& /*value*/) {}

}
