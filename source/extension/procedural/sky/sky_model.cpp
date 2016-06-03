#include "sky_model.hpp"
#include "hosek/ArHosekSkyModel.h"
#include "base/math/vector.inl"
#include "base/spectrum/spectrum.inl"

#include <iostream>

namespace procedural { namespace sky {

Model::Model() : dirty_(true) {
	for (uint32_t i = 0; i < Num_bands; ++i) {
		skymodel_states_[i] = nullptr;
	}

	Spectrum::init(380.f, 720.f);
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

	/*
	for (uint32_t i = 0; i < 3; ++i) {
		skymodel_states_[i] = arhosek_rgb_skymodelstate_alloc_init(turbidity_,
																   ground_albedo_.v[i],
																   elevation);
	}
	*/

	for (uint32_t i = 0; i < Num_bands; ++i) {
		skymodel_states_[i] = arhosekskymodelstate_alloc_init(elevation,
															  turbidity_,
															  ground_albedo_.v[0]);
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

	/*
	math::float3 radiance;
	for (uint32_t i = 0; i < 3; ++i) {
		radiance.v[i] = static_cast<float>(arhosek_tristim_skymodel_radiance(skymodel_states_[i],
																			 theta, gamma, i));
	}
	*/

	Spectrum radiance;
	for (uint32_t i = 0; i < Num_bands; ++i) {
		float wl_center = Spectrum::wavelength_center(i);
		radiance.set_bin(i, static_cast<float>(arhosekskymodel_radiance(skymodel_states_[i],
																		theta, gamma,
																		wl_center)));
	}

	return spectrum::XYZ_to_linear_RGB(radiance.XYZ());
}

void Model::release() {
	for (uint32_t i = 0; i < Num_bands; ++i) {
		arhosekskymodelstate_free(skymodel_states_[i]);
	}
}

const math::float3 Model::zenith_ = math::float3(0.f, 1.f, 0.f);

}}
