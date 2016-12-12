#include "material_test.hpp"
#include "bxdf.hpp"
#include "base/math/vector.inl"
#include "base/math/print.hpp"
#include <iostream>

namespace scene { namespace material { namespace testing {

void print_vector(float3_p v);

bool check(float3_p result, float3_p h,
		   float n_dot_wi, float n_dot_wo, float wo_dot_h, float pdf,
		   const Sample::Layer& layer) {
	if (!std::isfinite(pdf)
	||  !math::contains_only_finite(result)) {
		std::cout << "h: "; print_vector(h);
		std::cout << "n_dot_wi: " << n_dot_wi << std::endl;
		std::cout << "n_dot_wo: " << n_dot_wo << std::endl;
		std::cout << "wo_dot_h: " << wo_dot_h << std::endl;
		std::cout << "pdf: " << pdf << std::endl;
		std::cout << "reflection " << result << std::endl;
		std::cout << "t "; print_vector(layer.t_);
		std::cout << "b "; print_vector(layer.b_);
		std::cout << "n "; print_vector(layer.n_);
		return false;
	}

	return true;
}

bool check(const bxdf::Result& result, float3_p wo, const Sample::Layer& layer) {
	if (!std::isfinite(result.pdf)
	||  !math::contains_only_finite(result.reflection)) {
		std::cout << "wi: "; print_vector(result.wi);
		std::cout << "wo: "; print_vector(wo);
		std::cout << "h: "; print_vector(result.h);
	//	std::cout << "n_dot_wi: " << n_dot_wi << std::endl;
	//	std::cout << "n_dot_wo: " << n_dot_wo << std::endl;
	//	std::cout << "wo_dot_h: " << wo_dot_h << std::endl;
		std::cout << "pdf: " << result.pdf << std::endl;
		std::cout << "reflection " << result.reflection << std::endl;
		std::cout << "t "; print_vector(layer.t_);
		std::cout << "b "; print_vector(layer.b_);
		std::cout << "n "; print_vector(layer.n_);
		return false;
	}

	return true;
}

bool check_normal_map(float3_p n, float3_p tangent_space_n, float2 uv) {
	if (!math::contains_only_finite(n)) {
		std::cout << "n: "; print_vector(n);
		std::cout << "ts_n: "; print_vector(tangent_space_n);
		std::cout << "uv: " << uv << std::endl;
		return false;
	}

	return true;
}

void print_vector(float3_p v) {
	std::cout << v << " |" << math::length(v) << "|" << std::endl;
}

}}}
