#pragma once

#ifdef SU_DEBUG

#include "scene/material/bxdf.hpp"
#include "scene/material/material_test.hpp"
#include "base/math/print.hpp"
#include <iostream>

namespace scene { namespace material { namespace ggx {

template<typename Layer>
bool check(const bxdf::Result& result, const float3& wo,
		   float n_dot_wi, float n_dot_wo, float wo_dot_h,
		   const Layer& layer, float2 xi) {
	using namespace testing;

	if (!std::isfinite(result.pdf)
	||  !math::all_finite(result.wi)
	||  !math::all_finite(result.reflection)) {
		std::cout << "wi: "; print_vector(result.wi);
		std::cout << "wo: "; print_vector(wo);
		std::cout << "h: "; print_vector(result.h);
		std::cout << "n_dot_wi: " << n_dot_wi << std::endl;
		std::cout << "n_dot_wo: " << n_dot_wo << std::endl;
		std::cout << "wo_dot_h: " << wo_dot_h << std::endl;
		std::cout << "pdf: " << result.pdf << std::endl;
		std::cout << "reflection: " << result.reflection << std::endl;
		std::cout << "t: "; print_vector(layer.t_);
		std::cout << "b: "; print_vector(layer.b_);
		std::cout << "n: "; print_vector(layer.n_);
		std::cout << "alpha: " << layer.alpha_ << std::endl;
		std::cout << "xi: " << xi << std::endl;
		return false;
	}

	return true;
}

}}}

#endif
