#include "grid.hpp"
#include "image/texture/texture.hpp"
#include "image/texture/texture_adapter.inl"
#include "image/texture/sampler/address_mode.hpp"
#include "image/texture/sampler/sampler_nearest_3d.inl"
#include "image/texture/sampler/sampler_linear_3d.inl"
#include "scene/scene_worker.hpp"
#include "base/json/json.hpp"
#include "base/math/aabb.inl"
#include "base/math/vector.inl"

namespace scene { namespace volume {

Grid::Grid(Texture_ptr grid) : grid_(grid) {}

float Grid::density(float3_p p, Worker& worker, Sampler_filter filter) const {
	// p is in object space already

	if (!local_aabb_.intersect(p)) {
		return 0.f;
	}

	float3 p_g = 0.5f * (float3(1.f) + p);
	p_g[1] = 1.f - p_g[1];

	auto& sampler = worker.sampler_3D(static_cast<uint32_t>(Sampler_filter::Linear), filter);

	float density = grid_.sample_1(sampler, p_g);

	return density;
}

void Grid::set_parameter(const std::string& /*name*/, const json::Value& /*value*/) {}

}}
