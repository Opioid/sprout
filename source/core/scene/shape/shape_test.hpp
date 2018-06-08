#ifndef SU_CORE_SCENE_SHAPE_TEST_HPP
#define SU_CORE_SCENE_SHAPE_TEST_HPP

#include "base/math/vector2.hpp"

namespace scene {

namespace entity {
struct Composed_transformation;
}

struct Ray;

namespace shape {

struct Intersection;
struct Sample;

namespace testing {

bool check(Intersection const& intersection, const entity::Composed_transformation& transformation,
           Ray const& ray);

bool check(const Sample& sample);

bool check(const Sample& sample, float2 uv);

void test();

}  // namespace testing
}  // namespace shape
}  // namespace scene

#endif
