#include "grid.hpp"
#include "image/texture/texture.hpp"
#include "image/texture/texture_adapter.inl"
#include "image/texture/sampler/address_mode.hpp"
#include "image/texture/sampler/sampler_3d_nearest.inl"
#include "base/json/json.hpp"
#include "base/math/vector.inl"
#include "base/math/bounding/aabb.inl"

namespace scene { namespace volume {

Grid::Grid(Texture_ptr grid) : grid_(grid) {}

float Grid::density(float3_p p) const {
	float3 grid_p = 0.5f * (float3(1.f) + (p - scene_bb_.position()) / scene_bb_.halfsize());
	grid_p.y = 1.f - grid_p.y;

	image::texture::sampler::Sampler_3D_nearest<image::texture::sampler::Address_mode_repeat> sampler;

	float density = grid_.sample_1(sampler, grid_p);

	return density;
}

void Grid::set_parameter(const std::string& /*name*/,
						 const json::Value& /*value*/) {}

}}
