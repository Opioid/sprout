#ifndef SU_CORE_SCENE_MATERIAL_PRINT_HPP
#define SU_CORE_SCENE_MATERIAL_PRINT_HPP

#include "base/math/print.hpp"
#include "scene/material/bxdf.hpp"

#include <iostream>

namespace scene::material {

inline void print(const bxdf::Result& result) {
    std::cout << "reflection " << result.reflection << std::endl;
    std::cout << "pdf " << result.pdf() << std::endl;
}

inline void print(const bxdf::Sample& result) {
    std::cout << "wi " << result.wi << std::endl;
    std::cout << "reflection " << result.reflection << std::endl;
    std::cout << "pdf " << result.pdf << std::endl;
}

}  // namespace scene::material

#endif
