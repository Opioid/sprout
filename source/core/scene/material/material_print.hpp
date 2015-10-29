#pragma once

#include "base/math/print.hpp"
#include <iostream>

namespace scene { namespace material {

void print(const math::float3& reflection, float pdf) {
	std::cout << "reflection " << reflection << std::endl;
	std::cout << "pdf " << pdf << std::endl;
}

void print(const BxDF_result& result) {
	std::cout << "wi " << result.wi << std::endl;
	std::cout << "reflection " << result.reflection << std::endl;
}

}}
