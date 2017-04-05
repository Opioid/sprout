#pragma once

#include "base/math/vector3.hpp"

namespace sampler { class Sampler; }

namespace scene { namespace material {

class Sample;

namespace bxdf { struct Result; }

namespace ggx {

float clamp_roughness(float roughness);

float map_roughness(float roughness);

class Isotropic {

public:

	template<typename Layer, typename Fresnel>
	static float3 reflection(float n_dot_wi, float n_dot_wo, float wo_dot_h, float n_dot_h,
							 const Layer& layer, const Fresnel& fresnel, float& pdf);

	template<typename Layer, typename Fresnel>
	static float reflect(const float3& wo, float n_dot_wo,
						 const Layer& layer, const Fresnel& fresnel,
						 sampler::Sampler& sampler, bxdf::Result& result);

	template<typename Layer, typename Fresnel>
	static float3 refraction(const float3& wi, const float3& wo, float n_dot_wi,
							 float n_dot_wo, float n_dot_t, const Layer& layer,
							 const Fresnel& fresnel, float& pdf);

	template<typename Layer, typename Fresnel>
	static float refract(const float3& wo, float n_dot_wo, float n_dot_t, const Layer& layer,
						 const Fresnel& fresnel, sampler::Sampler& sampler, bxdf::Result& result);

	template<typename Layer, typename Fresnel>
	static float3 reflection(float n_dot_wi, float n_dot_wo, float wo_dot_h, float n_dot_h,
							 const Layer& layer, const Fresnel& fresnel,
							 float3& fresnel_result, float& pdf);

	template<typename Layer, typename Fresnel>
	static float reflect(const float3& wo, float n_dot_wo, const Layer& layer,
						 const Fresnel& fresnel, sampler::Sampler& sampler,
						 float3& fresnel_result, bxdf::Result& result);
};

class Anisotropic {

public:

	template<typename Layer, typename Fresnel>
	static float3 reflection(const float3& h, float n_dot_wi, float n_dot_wo, float wo_dot_h,
							 const Layer& layer, const Fresnel& fresnel, float& pdf);

	template<typename Layer, typename Fresnel>
	static float reflect(const float3& wo, float n_dot_wo,
						 const Layer& layer, const Fresnel& fresnel,
						 sampler::Sampler& sampler, bxdf::Result& result);
};

}}}
