#include "substitute_translucent_sample.hpp"
#include "rendering/integrator/surface/integrator_helper.hpp"
#include "scene/material/bxdf.hpp"
#include "scene/material/material_sample.inl"
#include "scene/material/lambert/lambert.inl"
#include "sampler/sampler.hpp"
#include "base/math/vector.inl"
#include "base/math/math.hpp"
#include "base/math/sampling/sampling.inl"

namespace scene { namespace material { namespace substitute {

float3 Sample_translucent::evaluate(float3_p wi, float& pdf) const {
	if (!same_hemisphere(wo_)) {
		pdf = 0.f;
		return math::float3_identity;
	}

	// This is a bit complicated to understand:
	// If the material does not have transmission, we will never get a wi which is in the wrong hemisphere,
	// because that case is handled before coming here,
	// so the check is only neccessary for transmissive materials (codified by thickness > 0).
	// On the other hand, if the there is transmission and wi is actullay coming from "behind",
	// then we don't need to calculate the reflection.
	// In the other case, transmission won't be visible and we only need reflection.
	if (thickness_ > 0.f && !same_hemisphere(wi)) {
		float n_dot_wi = std::max(-math::dot(layer_.n, wi), 0.00001f);
		float approximated_distance = thickness_ / n_dot_wi;
		float3 attenuation = rendering::attenuation(approximated_distance, attenuation_);
		pdf = 0.5f * n_dot_wi * math::Pi_inv;
		return n_dot_wi * (math::Pi_inv * attenuation * layer_.diffuse_color);
	}

	float3 result = layer_.base_evaluate(wi, wo_, pdf);

	if (thickness_ > 0.f) {
		pdf *= 0.5f;
	}

	return result;
}

void Sample_translucent::sample_evaluate(sampler::Sampler& sampler, bxdf::Result& result) const {
	if (!same_hemisphere(wo_)) {
		result.pdf = 0.f;
		return;
	}

	if (thickness_ > 0.f) {
		float p = sampler.generate_sample_1D();

		if (p < 0.5f) {
			float n_dot_wi = lambert::Isotropic::importance_sample(layer_.diffuse_color, layer_,
																   sampler, result);
			result.wi *= -1.f;
			result.pdf *= 0.5f;
			float approximated_distance = thickness_ / n_dot_wi;
			float3 attenuation = rendering::attenuation(approximated_distance, attenuation_);
			result.reflection *= n_dot_wi * attenuation;
		} else {
			if (1.f == layer_.metallic) {
				layer_.pure_specular_importance_sample(wo_, sampler, result);
			} else {
				if (p < 0.75f) {
					layer_.diffuse_importance_sample(wo_, sampler, result);
				} else {
					layer_.specular_importance_sample(wo_, sampler, result);
				}
			}

			result.pdf *= 0.5f;
		}
	} else {
		if (1.f == layer_.metallic) {
			layer_.pure_specular_importance_sample(wo_, sampler, result);
		} else {
			float p = sampler.generate_sample_1D();

			if (p < 0.5f) {
				layer_.diffuse_importance_sample(wo_, sampler, result);
			} else {
				layer_.specular_importance_sample(wo_, sampler, result);
			}
		}
	}
}

bool Sample_translucent::is_translucent() const {
	return thickness_ > 0.f;
}

void Sample_translucent::set(float3_p color, float metallic,
							 float thickness, float attenuation_distance) {
	float3 diffuse_color = (1.f - metallic) * color;

	if (thickness > 0.f) {
		attenuation_ = material::Sample::attenuation(diffuse_color, attenuation_distance);
	}
}

}}}
