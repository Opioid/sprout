#include "sky_model.hpp"
#include "hosek/ArHosekSkyModel.h"
#include "base/math/vector.inl"

namespace procedural { namespace sky {

Model::Model() : dirty_(true) {
	for (uint32_t i = 0; i < 3; ++i) {
		skymodel_states_[i] = nullptr;
	}
}

Model::~Model() {
	release();
}

void Model::init() {
	if (!dirty_) {
		return;
	}

	release();

	float elevation = std::max(math::dot(sun_direction_, zenith_) * -0.5f * math::Pi, 0.f);

	for (uint32_t i = 0; i < 3; ++i) {
		skymodel_states_[i] = arhosek_rgb_skymodelstate_alloc_init(turbidity_,
																   ground_albedo_.v[i],
																   elevation);
	}

	dirty_ = false;
}

void Model::set_sun_direction(math::pfloat3 direction) {
	sun_direction_ = direction;
	dirty_ = true;
}

void Model::set_ground_albedo(math::pfloat3 albedo) {
	ground_albedo_ = albedo;
	dirty_ = true;
}

void Model::set_turbidity(float turbidity) {
	turbidity_ = turbidity;
	dirty_ = true;
}

math::float3 Model::evaluate(math::pfloat3 wi) const {
	float wi_dot_z = std::max(wi.y, 0.00001f);
	float wi_dot_s = std::min(-math::dot(wi, sun_direction_), 0.99999f);

	float theta = std::acos(wi_dot_z);
	float gamma = std::acos(wi_dot_s);

	math::float3 radiance;
	for (uint32_t i = 0; i < 3; ++i) {
		radiance.v[i] = static_cast<float>(arhosek_tristim_skymodel_radiance(skymodel_states_[i],
																			 theta, gamma, i));
	}

	return radiance;
}

void Model::release() {
	for (uint32_t i = 0; i < 3; ++i) {
		arhosekskymodelstate_free(skymodel_states_[i]);
	}
}

const math::float3 Model::zenith_ = math::float3(0.f, 1.f, 0.f);

}}
