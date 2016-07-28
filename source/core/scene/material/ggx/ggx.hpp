#pragma once

#include "base/math/vector.hpp"

namespace sampler { class Sampler; }

namespace scene { namespace material {

class Sample;

namespace bxdf { struct Result; }

namespace ggx {

class Isotropic {

public:

	template<typename Layer, typename Fresnel>
	static float3 reflection(float3_p wi, float3_p wo, float n_dot_wi, float n_dot_wo,
							 const Layer& layer, const Fresnel& fresnel, float& pdf);

	template<typename Layer, typename Fresnel>
	static float reflect(float3_p wo, float n_dot_wo, const Layer& layer, const Fresnel& fresnel,
						 sampler::Sampler& sampler, bxdf::Result& result);

	template<typename Layer, typename Fresnel>
	static float3 refraction(float3_p wi, float3_p wo, float n_dot_wi,
							 float n_dot_wo, float n_dot_t, const Layer& layer,
							 const Fresnel& fresnel, float& pdf);

	template<typename Layer, typename Fresnel>
	static float refract(float3_p wo, float n_dot_wo, float n_dot_t, const Layer& layer,
						 const Fresnel& fresnel, sampler::Sampler& sampler, bxdf::Result& result);

	template<typename Layer, typename Fresnel>
	static float3 reflection(float3_p wi, float3_p wo, float n_dot_wi, float n_dot_wo,
							 const Layer& layer, const Fresnel& fresnel,
							 float3& fresnel_result, float& pdf);

	template<typename Layer, typename Fresnel>
	static float reflect(float3_p wo, float n_dot_wo, const Layer& layer, const Fresnel& fresnel,
						 sampler::Sampler& sampler, float3& fresnel_result, bxdf::Result& result);
};

class Anisotropic {

public:

	template<typename Layer, typename Fresnel>
	static float3 reflection(float3_p wi, float3_p wo, float n_dot_wi, float n_dot_wo,
							 const Layer& layer, const Fresnel& fresnel, float& pdf);

	template<typename Layer, typename Fresnel>
	static float reflect(float3_p wo, float n_dot_wo, const Layer& layer, const Fresnel& fresnel,
						 sampler::Sampler& sampler, bxdf::Result& result);
};

}}}
