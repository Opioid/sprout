#include "shape_test.hpp"
#include "infinite_sphere.hpp"
#include "sphere.hpp"
#include "shape_intersection.hpp"
#include "node_stack.inl"
#include "scene/scene_ray.inl"
#include "scene/entity/composed_transformation.hpp"
#include "base/math/vector.inl"
#include "base/math/matrix.inl"
#include "base/math/print.hpp"
#include <iostream>

namespace scene { namespace shape { namespace testing {

void print(const Intersection& intersection);

void test() {
	std::cout << "scene::shape::testing::test()" << std::endl;

	Node_stack node_stack(0);

	math::transformation transformation;
	transformation.position = float3(0.f);
	transformation.scale = float3(1.f);
	transformation.rotation = math::quaternion_identity;

	entity::Composed_transformation composed_transformation;
	composed_transformation.set(transformation);

	scene::Ray ray;
	ray.origin = float3(0.f, 4.f, 0.f);
	ray.set_direction(float3(0.f, -1.f, 0.f));
	ray.max_t = 1000000.f;
	ray.min_t = 0.f;

	Intersection intersection;

//	{
//		Infinite_sphere infinite_sphere;
//		if (infinite_sphere.intersect(composed_transformation, ray, node_stack, intersection)) {
//			print(intersection);
//		}
//	}

//	{
//		ray.origin = float3(0.f, 4.f, 0.f);
//		ray.set_direction(float3(0.f, -1.f, 0.f));

//		Sphere sphere;
//		if (sphere.intersect(composed_transformation, ray, node_stack, intersection)) {
//			print(intersection);
//		}
//	}

	{
		ray.origin = float3(0.f, 0.f, 0.f);
		ray.set_direction(float3(0.f, -1.f, 0.f));

		Sphere sphere;
		if (sphere.intersect(composed_transformation, ray, node_stack, intersection)) {
			print(intersection);
		}
	}
}

void print(const Intersection& intersection) {
	std::cout << "n: " << intersection.n << std::endl;
	std::cout << "t: " << intersection.t << std::endl;
	std::cout << "b: " << intersection.b << std::endl;
	std::cout << "uv: " << intersection.uv << std::endl;
}

}}}
