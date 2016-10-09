#include "testing_size.hpp"
#include "image/texture/texture_adapter.hpp"
#include "scene/entity/keyframe.hpp"
#include "scene/entity/composed_transformation.hpp"
#include "scene/shape/triangle/triangle_primitive_mte.hpp"
#include "scene/shape/triangle/bvh/triangle_bvh_node.inl"
#include "scene/shape/triangle/triangle_primitive_mt.hpp"
#include "base/math/vector.inl"
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

	print_size<float3>("float3", 16);

	print_size<float4>("float4", 16);

	print_size<math::Matrix3x3f_a>("float3x3", 48);

	print_size<math::float4x4>("float4x4", 64);

	print_size<math::transformation>("transformation", 48);

	print_size<scene::entity::Keyframe>("Keyframe", 64);

	print_size<scene::entity::Composed_transformation>("Composed_transformation", 208);

	print_size<scene::shape::triangle::Intersection_triangle_MTE>("Intersection_triangle_MTE", 64);

	print_size<scene::shape::triangle::Shading_triangle_MTE>("Shading_triangle_MTE", 128);

	print_size<scene::shape::triangle::bvh::Node>("triangle::bvh::Node", 32);

	print_size<scene::shape::triangle::Intersection_vertex_MT>("Intersection_vertex_MT", 16);

	print_size<scene::shape::triangle::Shading_vertex_MT>("Shading_vertex_MT", 48);

	print_size<image::texture::Adapter>("texture::Adapter", 24);
}

}
