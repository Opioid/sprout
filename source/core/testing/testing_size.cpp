#include "testing_size.hpp"
#include "scene/entity/keyframe.hpp"
#include "scene/entity/composed_transformation.hpp"
#include "scene/shape/triangle/triangle_primitive_mte.hpp"
#include <string>
#include <iostream>

namespace testing {

template<typename T>
void print_size(const std::string& name, size_t expected_size) {
	std::cout << "sizeof(" << name << ") == " << sizeof(T)
			  << " (" << expected_size << ")" << std::endl;
}

void size() {
	std::cout << "Sizes:" << std::endl;

	print_size<math::float3>("float3", 16);

	print_size<math::float4x4>("float4x4", 64);

	print_size<math::transformation>("transformation", 48);

	print_size<scene::entity::Keyframe>("Keyframe", 64);

	print_size<scene::entity::Composed_transformation>("Composed_transformation", 224);

	print_size<scene::shape::triangle::Intersection_triangle_MTE>("Intersection_triangle_MTE", 64);

	print_size<scene::shape::triangle::Shading_triangle_MTE>("Shading_triangle_MTE", 128);
}

}
