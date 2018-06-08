#pragma once

#include <iostream>
#include "base/math/print.hpp"
#include "scene/material/bxdf.hpp"

namespace scene {
namespace material {

inline void print(const bxdf::Result& result) {
    std::cout << "reflection " << result.reflection << std::endl;
    std::cout << "pdf " << result.pdf << std::endl;
}

inline void print(const bxdf::Sample& result) {
    std::cout << "wi " << result.wi << std::endl;
    std::cout << "reflection " << result.reflection << std::endl;
    std::cout << "pdf " << result.pdf << std::endl;
}

}  // namespace material
}  // namespace scene
