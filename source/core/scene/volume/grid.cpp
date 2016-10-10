#include "grid.hpp"
#include "image/texture/texture.hpp"
#include "image/texture/texture_adapter.inl"
#include "image/texture/sampler/address_mode.hpp"
#include "image/texture/sampler/sampler_3d_nearest.inl"
#include "image/texture/sampler/sampler_3d_linear.inl"
#include "scene/scene_worker.hpp"
#include "base/json/json.hpp"
#include "base/math/vector.inl"
#include "base/math/bounding/aabb.inl"

namespace scene { namespace volume {

Grid::Grid(Texture_ptr grid) : grid_(grid) {}

float Grid::density(float3_p p, Worker& worker, Sampler_filter filter) const {
	float3 grid_p = 0.5f * (float3(1.f) + (p - scene_bb_.position()) / scene_bb_.halfsize());
	grid_p.y = 1.f - grid_p.y;

	auto& sampler = worker.sampler_3D(static_cast<uint32_t>(Sampler_filter::Linear), filter);

	float density = grid_.sample_1(sampler, grid_p);

	return density;
}

void Grid::set_parameter(const std::string& /*name*/,
						 const json::Value& /*value*/) {}

}}
