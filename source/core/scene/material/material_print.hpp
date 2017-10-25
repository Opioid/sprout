#pragma once

#include "scene/material/bxdf.hpp"
#include "base/math/print.hpp"
#include <iostream>

namespace scene { namespace material {

inline void print(const float3& reflection, float pdf) {
	std::cout << "reflection " << reflection << std::endl;
	std::cout << "pdf " << pdf << std::endl;
}

inline void print(const bxdf::Sample& result) {
	std::cout << "wi " << result.wi << std::endl;
	std::cout << "reflection " << result.reflection << std::endl;
	std::cout << "pdf " << result.pdf << std::endl;
}

}}
