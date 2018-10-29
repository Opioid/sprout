#include "shape_test.hpp"
#include <iostream>
#include "base/math/matrix4x4.inl"
#include "base/math/print.hpp"
#include "base/math/quaternion.inl"
#include "base/math/vector3.inl"
#include "infinite_sphere.hpp"
#include "node_stack.inl"
#include "scene/entity/composed_transformation.hpp"
#include "scene/scene_constants.hpp"
#include "scene/scene_ray.inl"
#include "shape_intersection.hpp"
#include "shape_sample.hpp"
#include "sphere.hpp"

namespace scene::shape::testing {

void print(Intersection const& intersection);

void print_vector(float3 const& v);

bool check(Intersection const& intersection,
           const entity::Composed_transformation& /*transformation*/, Ray const& /*ray*/) {
    if (!std::isfinite(math::length(intersection.b))) {
        print(intersection);

        return false;
    }

    return true;
}

bool check(const Sample_to& sample) {
    if (sample.pdf <= 0.f) {
        std::cout << "pdf " << sample.pdf << std::endl;
        return false;
    }

    if (!math::all_finite(sample.uv)) {
        return false;
    }

    return true;
}

bool check(const Sample_to& sample, float2 uv) {
    if (sample.pdf <= 0.f) {
        std::cout << "pdf " << sample.pdf << std::endl;
        return false;
    }

    if (!math::all_finite(sample.uv)) {
        std::cout << "uv " << uv << std::endl;
        return false;
    }

    return true;
}

void test() {
    std::cout << "scene::shape::testing::test()" << std::endl;

    Node_stack node_stack(0);

    math::Transformation transformation;
    transformation.position = float3(0.f);
    transformation.scale    = float3(1.f);
    transformation.rotation = math::quaternion::identity();

    entity::Composed_transformation composed_transformation;
    composed_transformation.set(transformation);

    scene::Ray ray;
    ray.origin = float3(0.f, 4.f, 0.f);
    ray.set_direction(float3(0.f, -1.f, 0.f));
    ray.max_t = scene::Ray_max_t;
    ray.min_t = 0.f;

    Intersection intersection;

    //	{
    //		Infinite_sphere infinite_sphere;
    //		if (infinite_sphere.intersect(composed_transformation, ray, node_stack,
    // intersection)) { 			print(intersection);
    //		}
    //	}

    {
        ray.origin = float3(0.f, 4.f, 0.f);
        ray.set_direction(float3(0.f, -1.f, 0.f));

        Sphere sphere;
        if (sphere.intersect(ray, composed_transformation, node_stack, intersection)) {
            print(intersection);
        }
    }

    //	{
    //		ray.origin = float3(0.f, 0.f, 0.f);
    //		ray.set_direction(float3(0.f, -1.f, 0.f));

    //		Sphere sphere;
    //		if (sphere.intersect(composed_transformation, ray, node_stack, intersection)) {
    //			print(intersection);
    //		}
    //	}
}

void print(Intersection const& intersection) {
    std::cout << "n: ";
    print_vector(intersection.n);
    std::cout << "t: ";
    print_vector(intersection.t);
    std::cout << "b: ";
    print_vector(intersection.b);
    std::cout << "uv: " << intersection.uv << std::endl;
}

void print_vector(float3 const& v) {
    std::cout << v << " |" << math::length(v) << "|" << std::endl;
}

}  // namespace scene::shape::testing
