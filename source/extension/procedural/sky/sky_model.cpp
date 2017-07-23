#include "sky_model.hpp"
#include "hosek/ArHosekSkyModel.hpp"
#include "base/math/vector3.inl"
#include "base/spectrum/discrete.inl"
#include "base/spectrum/xyz.hpp"

namespace procedural { namespace sky {

Model::Model() : dirty_(true) {
	for (uint32_t i = 0; i < Num_bands; ++i) {
		skymodel_states_[i] = nullptr;
	}

	Spectrum::init(380.f, 720.f);
//	Spectrum::init(410.f, 690.f);
}

Model::~Model() {
	release();
}

bool Model::init() {
	if (!dirty_) {
		return false;
	}

	release();

	const float elevation = std::max(math::dot(sun_direction_, zenith_) * (-0.5f * math::Pi), 0.f);

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
															  ground_albedo_[0]);
	}

	dirty_ = false;

	return true;
}

float3 Model::sun_direction() const {
	return sun_direction_;
}

void Model::set_sun_direction(const float3& direction) {
	sun_direction_ = direction;
	dirty_ = true;
}

void Model::set_ground_albedo(const float3& albedo) {
	ground_albedo_ = albedo;
	dirty_ = true;
}

void Model::set_turbidity(float turbidity) {
	turbidity_ = turbidity;
	dirty_ = true;
}

float3 Model::evaluate_sky(const float3& wi) const {
	const float wi_dot_z = std::max(wi[1], 0.00001f);
	const float wi_dot_s = std::min(-math::dot(wi, sun_direction_), 0.99999f);

	const float theta = std::acos(wi_dot_z);
	const float gamma = std::acos(wi_dot_s);

	/*
	float3 radiance;
	for (uint32_t i = 0; i < 3; ++i) {
		radiance.v[i] = static_cast<float>(arhosek_tristim_skymodel_radiance(skymodel_states_[i],
																			 theta, gamma, i));
	}
	*/

	Spectrum radiance;
	for (uint32_t i = 0; i < Num_bands; ++i) {
		const float wl_center = Spectrum::wavelength_center(i);
		radiance.set_bin(i, static_cast<float>(arhosekskymodel_radiance(skymodel_states_[i],
																		theta, gamma,
																		wl_center)));
	}

	return spectrum::XYZ_to_linear_RGB(radiance.XYZ());
}

float3 Model::evaluate_sky_and_sun(const float3& wi) const {
	const float wi_dot_z = std::max(wi[1], 0.00001f);
	const float wi_dot_s = std::min(-math::dot(wi, sun_direction_), 0.99999f);

	const float theta = std::acos(wi_dot_z);
	const float gamma = std::acos(wi_dot_s);

	Spectrum radiance;
	for (uint32_t i = 0; i < Num_bands; ++i) {
		const float wl_center = Spectrum::wavelength_center(i);
		radiance.set_bin(i, static_cast<float>(arhosekskymodel_solar_radiance(skymodel_states_[i],
																			  theta, gamma,
																			  wl_center)));
	}

	return spectrum::XYZ_to_linear_RGB(radiance.XYZ());
}

float3 Model::zenith() {
	return zenith_;
}

void Model::release() {
	for (uint32_t i = 0; i < Num_bands; ++i) {
		arhosekskymodelstate_free(skymodel_states_[i]);
	}
}

const float3 Model::zenith_ = float3(0.f, 1.f, 0.f);

}}
