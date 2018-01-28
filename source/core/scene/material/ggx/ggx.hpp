#ifndef SU_CORE_SCENE_MATERIAL_GGX_HPP
#define SU_CORE_SCENE_MATERIAL_GGX_HPP

#include "base/math/vector3.hpp"

namespace sampler { class Sampler; }

namespace scene::material {

class Sample;

namespace bxdf { struct Result; struct Sample; }

namespace ggx {

class Isotropic {

public:

	template<typename Layer, typename Fresnel>
	static bxdf::Result reflection(float n_dot_wi, float n_dot_wo, float wo_dot_h, float n_dot_h,
								   const Layer& layer, const Fresnel& fresnel);

	template<typename Layer, typename Fresnel>
	static bxdf::Result reflection(float n_dot_wi, float n_dot_wo, float wo_dot_h, float n_dot_h,
								   const Layer& layer, const Fresnel& fresnel,
								   float3& fresnel_result);

	template<typename Layer, typename Fresnel>
	static float reflect(const float3& wo, float n_dot_wo, const Layer& layer,
						 const Fresnel& fresnel, sampler::Sampler& sampler, bxdf::Sample& result);

	template<typename Layer, typename Fresnel>
	static float reflect(const float3& wo, float n_dot_wo, const Layer& layer,
						 const Fresnel& fresnel, sampler::Sampler& sampler,
						 float3& fresnel_result, bxdf::Sample& result);

	template<typename Layer, typename Fresnel>
	static bxdf::Result refraction(float n_dot_wi,
								   float n_dot_wo, float wi_dot_h, float wo_dot_h, float n_dot_t,
								   const Layer& layer, const Fresnel& fresnel);

	template<typename Layer, typename Fresnel>
	static float refract(const float3& wo, float n_dot_wo, float n_dot_t, const Layer& layer,
						 const Fresnel& fresnel, sampler::Sampler& sampler, bxdf::Sample& result);

	template<typename Layer, typename IOR, typename Fresnel>
	static float refract(const float3& wo, float n_dot_wo, float n_dot_t,
						 const Layer& layer, const IOR& ior, const Fresnel& fresnel,
						 sampler::Sampler& sampler, bxdf::Sample& result);
};

class Anisotropic {

public:

	template<typename Layer, typename Fresnel>
	static bxdf::Result reflection(const float3& h, float n_dot_wi, float n_dot_wo, float wo_dot_h,
								   const Layer& layer, const Fresnel& fresnel);

	template<typename Layer, typename Fresnel>
	static float reflect(const float3& wo, float n_dot_wo, const Layer& layer,
						 const Fresnel& fresnel, sampler::Sampler& sampler, bxdf::Sample& result);
};

}}

#endif
