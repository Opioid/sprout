#include "shape_test.hpp"
#include "base/math/matrix4x4.inl"
#include "base/math/print.inl"
#include "base/math/quaternion.inl"
#include "base/math/vector3.inl"
#include "infinite_sphere.hpp"
#include "node_stack.inl"
#include "scene/composed_transformation.inl"
#include "scene/scene_constants.hpp"
#include "scene/scene_ray.inl"
#include "shape_intersection.hpp"
#include "shape_sample.hpp"
#include "sphere.hpp"

#include <iostream>

namespace scene::shape::testing {

void print(Intersection const& isec);

void print_vector(float3_p v);

bool check(Intersection const& isec, Composed_transformation const& trafo, Ray const& /*ray*/) {
    if (!std::isfinite(length(isec.b)) || !all_finite(isec.t) || !all_finite(isec.b) ||
        !all_finite(isec.n) || !all_finite(isec.geo_n)) {
        print(isec);

        std::cout << "t.rotation: " << trafo.rotation << std::endl;
        std::cout << "t.scale: " << trafo.scale() << std::endl;

        return false;
    }

    return true;
}

bool check(const Sample_to& sample) {
    if (sample.pdf() <= 0.f) {
        std::cout << "pdf " << sample.pdf() << std::endl;
        return false;
    }

    return all_finite(sample.uvw);
}

void print(Intersection const& isec) {
    std::cout << "n: ";
    print_vector(isec.n);
    std::cout << "t: ";
    print_vector(isec.t);
    std::cout << "b: ";
    print_vector(isec.b);
    std::cout << "uv: " << isec.uv << std::endl;
    std::cout << "geo_n: ";
    print_vector(isec.geo_n);
}

void print_vector(float3_p v) {
    std::cout << v << " |" << length(v) << "|" << std::endl;
}

}  // namespace scene::shape::testing
