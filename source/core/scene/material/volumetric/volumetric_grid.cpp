#include "volumetric_grid.hpp"
#include "image/texture/texture_adapter.inl"
#include "scene/scene_worker.hpp"
#include "scene/entity/composed_transformation.hpp"
#include "base/math/matrix4x4.inl"
#include "base/math/ray.inl"
#include "base/random/generator.inl"
#include "base/spectrum/rgb.hpp"

namespace scene::material::volumetric {

Grid::Grid(const Sampler_settings& sampler_settings, const Texture_adapter& grid) :
	Density(sampler_settings), grid_(grid) {
	calculate_max_extinction();
}

Grid::~Grid() {}

float Grid::max_extinction() const {
	return max_density_ * spectrum::average(absorption_coefficient_ + scattering_coefficient_);
}

bool Grid::is_heterogeneous_volume() const {
	return true;
}

size_t Grid::num_bytes() const {
	return sizeof(*this);
}

float Grid::density(const Transformation& /*transformation*/, const float3& p,
					Sampler_filter filter, const Worker& worker) const {
	// p is in object space already

	float3 p_g = 0.5f * (float3(1.f) + p);
	p_g[1] = 1.f - p_g[1];

	const auto& sampler = worker.sampler_3D(sampler_key(), filter);

	return grid_.sample_1(sampler, p_g);
}

void Grid::calculate_max_extinction() {
	float max_density = 0.f;

	const auto texture = grid_.texture();

	const int3 d = texture->dimensions_3();

	for (uint32_t i = 0, len = d[0] * d[1] * d[2]; i < len; ++i) {
		max_density = std::max(texture->at_1(i), max_density);
	}

	max_density_ = max_density;
}

Emission_grid::Emission_grid(const Sampler_settings& sampler_settings,
							 const Texture_adapter& grid) :
	Material(sampler_settings), grid_(grid) {}

Emission_grid::~Emission_grid() {}

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
									const math::AABB& /*aabb*/, const math::Ray& /*ray*/,
									float /*step_size*/, rnd::Generator& /*rng*/,
									Sampler_filter /*filter*/, const Worker& /*worker*/) const {
	return float3::identity();
}

float3 Emission_grid::scattering(const Transformation& /*transformation*/, const float3& /*p*/,
								 Sampler_filter /*filter*/, const Worker& /*worker*/) const {
	return float3::identity();
}

size_t Emission_grid::num_bytes() const {
	return sizeof(*this);
}

float3 Emission_grid::emission(const float3& p, Sampler_filter filter, const Worker& worker) const {
	// p is in object space already

	float3 p_g = 0.5f * (float3(1.f) + p);
	p_g[1] = 1.f - p_g[1];

	const auto& sampler = worker.sampler_3D(sampler_key(), filter);

	return 0.00001f * grid_.sample_3(sampler, p_g);
}

}
