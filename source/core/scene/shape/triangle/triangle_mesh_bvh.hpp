#ifndef SU_CORE_SCENE_SHAPE_TRIANGLE_MESH_BVH_HPP
#define SU_CORE_SCENE_SHAPE_TRIANGLE_MESH_BVH_HPP

#include "bvh/triangle_bvh_indexed_data.hpp"
#include "bvh/triangle_bvh_tree.hpp"

namespace scene::shape::triangle {

using Shading_vertex_type = struct Shading_vertex_MTC;

extern template class bvh::Tree<bvh::Indexed_data<Shading_vertex_type>>;

//	using Tree = bvh::Tree<bvh::Data_interleaved<Triangle_type>>;

//	using Tree = bvh::Tree<bvh::Data<Intersection_triangle_type, Shading_triangle_type>>;

using Tree = bvh::Tree<bvh::Indexed_data<Shading_vertex_type>>;

//	using Tree = bvh::Tree<bvh::Indexed_data_interleaved<Vertex_type>>;

//	using Tree = bvh::Tree<bvh::Hybrid_data<float3, Shading_vertex_type>>;

}  // namespace scene::shape::triangle

#endif
