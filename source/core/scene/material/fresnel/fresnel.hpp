#pragma once

#include "base/math/vector.hpp"

namespace scene { namespace material { namespace fresnel {

math::float3 schlick(float wo_dot_h, const math::float3& f0);

float schlick(float wo_dot_h, float f0);

float schlick_f0(float n0, float n1);

math::float3 conductor(float wo_dot_h, const math::float3& eta, const math::float3& k);

float dielectric(float n_dot_wi, float n_dot_wo, float eta);

float dielectric_holgerusan(float cos_theta_i, float cos_theta_t, float eta_i, float eta_t);

}}}
