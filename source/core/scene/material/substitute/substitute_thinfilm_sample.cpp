#include "substitute_thinfilm_sample.hpp"
#include "substitute_base_sample.inl"
#include "scene/material/material_sample.inl"
#include "scene/material/fresnel/fresnel.inl"
#include "base/math/vector.inl"
#include "base/math/math.hpp"

namespace scene { namespace material { namespace substitute {

math::float3 Sample_thinfilm::evaluate(math::pfloat3 wi, float& pdf) const {
	if (!same_hemisphere(wo_)) {
		pdf = 0.f;
		return math::float3_identity;
	}

	fresnel::Thinfilm_weighted thinfilm(1.f, coating_.ior, ior_,
										coating_.thickness, coating_.weight);

	return base_evaluate_and_coating(wi, thinfilm, coating_.a2, pdf);
}

void Sample_thinfilm::sample_evaluate(sampler::Sampler& sampler, bxdf::Result& result) const {
	if (!same_hemisphere(wo_)) {
		result.pdf = 0.f;
		return;
	}

	fresnel::Thinfilm_weighted thinfilm(1.f, coating_.ior, ior_,
										coating_.thickness, coating_.weight);

	base_sample_evaluate_and_coating(thinfilm, coating_.a2, sampler, result);
}

void Sample_thinfilm::set_thinfilm(const coating::Thinfilm& coating) {
	coating_ = coating;
}

}}}
