#ifndef SU_CORE_SCENE_SHAPE_TRIANGLE_PRIMITIVE_HPP
#define SU_CORE_SCENE_SHAPE_TRIANGLE_PRIMITIVE_HPP

#include <cstdint>

namespace scene::shape::triangle {

struct Index_triangle {
    Index_triangle() = default;

    Index_triangle(uint32_t a, uint32_t b, uint32_t c, uint32_t p) : i{a, b, c}, part(p) {}

    uint32_t i[3];

    uint32_t part;
};

}  // namespace scene::shape::triangle

#endif
