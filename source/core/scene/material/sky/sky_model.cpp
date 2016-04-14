#include "sky_model.hpp"
#include "hosek/ArHosekSkyModel.h"
#include "base/math/vector.inl"

#include <iostream>

namespace scene { namespace material { namespace sky {

Model::~Model() {
	for (uint32_t i = 0; i < 3; ++i) {
		arhosekskymodelstate_free(skymodel_states_[i]);
	}
}

void Model::init() {
	float solar_elevation = 0.5f * math::Pi;

	for (uint32_t i = 0; i < 3; ++i) {
		skymodel_states_[i] = arhosek_rgb_skymodelstate_alloc_init(turbidity_, ground_albedo_.v[i], solar_elevation);
	}
}

void Model::set_ground_albedo(math::pfloat3 albedo) {
	ground_albedo_ = albedo;
}

void Model::set_turbidity(float turbidity) {
	turbidity_ = turbidity;
}

math::float3 Model::evaluate(math::pfloat3 wi) const {

//	float theta = i_Direction.computeTheta();
//	double gamma = acosf(clamp(-dot(i_Direction, directionFromSun), -1.0f, 1.0f));

	float wi_dot_z = std::max(wi.y, 0.00001f);
	float wi_dot_s = std::max(math::dot(wi, math::float3(0.f, 1.f, 0.f)), 0.0f);

	float theta = std::acos(wi_dot_z);
	float gamma = std::acos(wi_dot_s);

	math::float3 radiance;
	for (uint32_t i = 0; i < 3; ++i) {
		radiance.v[i] = static_cast<float>(arhosek_tristim_skymodel_radiance(skymodel_states_[i], theta, gamma, i));
	}

	if (math::contains_inf(radiance) || math::contains_nan(radiance)) {
		std::cout << "sky error for wi_dot_z == " << wi_dot_z << " wi_dot_s == " << wi_dot_s << std::endl;
	}

	return radiance;
}

}}}
