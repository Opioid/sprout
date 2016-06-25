#include "substitute_clearcoat_sample.hpp"
#include "substitute_base_sample.inl"
#include "scene/material/material_sample.inl"
#include "scene/material/coating/coating.inl"
#include "scene/material/fresnel/fresnel.inl"
#include "base/math/vector.inl"
#include "base/math/math.hpp"

namespace scene { namespace material { namespace substitute {

float3 Sample_clearcoat::evaluate(float3_p wi, float& pdf) const {
	if (!same_hemisphere(wo_)) {
		pdf = 0.f;
		return math::float3_identity;
	}

	fresnel::Schlick_weighted clearcoat(coating_.f0, coating_.weight);

	return base_evaluate_and_coating(wi, clearcoat, coating_.a2, pdf);
}

void Sample_clearcoat::sample_evaluate(sampler::Sampler& sampler, bxdf::Result& result) const {
	if (!same_hemisphere(wo_)) {
		result.pdf = 0.f;
		return;
	}

	fresnel::Schlick_weighted clearcoat(coating_.f0, coating_.weight);

	base_sample_evaluate_and_coating(clearcoat, coating_.a2, sampler, result);
}

}}}
