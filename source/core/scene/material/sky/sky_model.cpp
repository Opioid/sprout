#include "sky_model.hpp"
#include "hosek/ArHosekSkyModel.h"
#include "base/math/vector.inl"

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
	float theta = std::acos(wi.y);
	float gamma = std::acos(math::dot(math::float3(0.f, 1.f, 0.f), wi));

	math::float3 radiance;
	for (uint32_t i = 0; i < 3; ++i) {
		radiance.v[i] = static_cast<float>(arhosek_tristim_skymodel_radiance(skymodel_states_[i], theta, gamma, i));
	}

	return radiance;
}

}}}
