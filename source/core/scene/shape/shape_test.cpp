#include "shape_test.hpp"
#include "base/math/matrix4x4.inl"
#include "base/math/print.hpp"
#include "base/math/quaternion.inl"
#include "base/math/vector3.inl"
#include "infinite_sphere.hpp"
#include "node_stack.inl"
#include "scene/entity/composed_transformation.inl"
#include "scene/scene_constants.hpp"
#include "scene/scene_ray.inl"
#include "shape_intersection.hpp"
#include "shape_sample.hpp"
#include "sphere.hpp"

#include <iostream>

namespace scene::shape::testing {

void print(Intersection const& isec);

void print_vector(float3_p v);

bool check(Intersection const& isec, entity::Composed_transformation const& trafo,
           Ray const& /*ray*/) {
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

void test() {
    std::cout << "scene::shape::testing::test()" << std::endl;

    Node_stack nodes;

    math::Transformation trafo;
    trafo.position = float3(0.f);
    trafo.scale    = float3(1.f);
    trafo.rotation = math::quaternion::Identity;

    entity::Composed_transformation composed_transformation;
    composed_transformation.set(trafo);

    scene::Ray ray;
    ray.origin = float3(0.f, 4.f, 0.f);
    ray.set_direction(float3(0.f, -1.f, 0.f));
    ray.max_t() = scene::Ray_max_t;
    ray.min_t() = 0.f;

    Intersection isec;

    //	{
    //		Infinite_sphere infinite_sphere;
    //		if (infinite_sphere.intersect(composed_transformation, ray, nodes,
    // isec)) { 			print(isec);
    //		}
    //	}

    {
        ray.origin = float3(0.f, 4.f, 0.f);
        ray.set_direction(float3(0.f, -1.f, 0.f));

        Sphere sphere;
        if (sphere.intersect(ray, composed_transformation, nodes, isec)) {
            print(isec);
        }
    }

    //	{
    //		ray.origin = float3(0.f, 0.f, 0.f);
    //		ray.set_direction(float3(0.f, -1.f, 0.f));

    //		Sphere sphere;
    //		if (sphere.intersect(composed_transformation, ray, nodes, isec)) {
    //			print(isec);
    //		}
    //	}
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
