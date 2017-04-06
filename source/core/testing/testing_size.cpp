#include "testing_size.hpp"
#include "image/texture/texture_adapter.hpp"
#include "scene/scene_renderstate.hpp"
#include "scene/bvh/scene_bvh_split_candidate.hpp"
#include "scene/scene_intersection.hpp"
#include "scene/scene_ray.inl"
#include "scene/bvh/scene_bvh_node.inl"
#include "scene/bvh/scene_bvh_builder.hpp"
#include "scene/entity/entity.hpp"
#include "scene/entity/keyframe.hpp"
#include "scene/entity/composed_transformation.hpp"
#include "scene/shape/shape_intersection.hpp"
#include "scene/shape/triangle/triangle_primitive_mt.hpp"
#include "base/math/vector3.inl"
#include <string>
#include <iostream>

namespace testing {

template<typename T>
void print_size(const std::string& name, size_t expected_size) {
	if (sizeof(T) != expected_size) {
		std::cout << "ALARM: ";
	}

	std::cout << "sizeof(" << name << ") == " << sizeof(T)
			  << " (" << expected_size << ")" << std::endl;
}

void size() {
	std::cout << "Sizes:" << std::endl;

	print_size<float3>("float3", 16);

	print_size<float4>("float4", 16);

	print_size<math::Matrix3x3f_a>("float3x3", 48);

	print_size<float4x4>("float4x4", 64);

	print_size<math::Transformation>("transformation", 48);

	print_size<scene::entity::Keyframe>("Keyframe", 64);

	print_size<scene::entity::Composed_transformation>("Composed_transformation", 208);

	print_size<math::Ray>("math::Ray", 64);
	print_size<scene::Ray>("scene::Ray", 80);

	print_size<scene::shape::Intersection>("shape::Intersection", 96);

	print_size<scene::Intersection>("scene::Intersection", 112);

	print_size<scene::entity::Entity>("Entity", 496);

	print_size<scene::Renderstate>("Renderstate", 112);

	print_size<scene::bvh::Node>("scene::bvh::Node", 32);

	print_size<scene::shape::triangle::Intersection_vertex_MT>("Intersection_vertex_MT", 16);

//	print_size<scene::shape::triangle::Shading_vertex_MT>("Shading_vertex_MT", 48);

	print_size<scene::shape::triangle::Shading_vertex_MTC>("Shading_vertex_MTC", 32);

	print_size<scene::bvh::Build_node>("bvh::Build_node", 64);

	print_size<image::texture::Adapter>("texture::Adapter", 32);

	print_size<scene::bvh::Split_candidate>("Split_candidate", 32);
}

}
