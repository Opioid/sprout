#ifndef SU_CORE_SCENE_SHAPE_TEST_HPP
#define SU_CORE_SCENE_SHAPE_TEST_HPP

#include "base/math/vector2.hpp"

namespace scene {

struct Composed_transformation;

struct Ray;

namespace shape {

struct Intersection;
struct Sample_to;

namespace testing {

bool check(Intersection const& isec, Composed_transformation const& trafo, Ray const& ray);

bool check(const Sample_to& sample);

}  // namespace testing
}  // namespace shape
}  // namespace scene

#endif
