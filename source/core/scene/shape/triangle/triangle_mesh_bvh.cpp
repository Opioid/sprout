#include "triangle_mesh_bvh.hpp"
#include "bvh/triangle_bvh_indexed_data.inl"
#include "bvh/triangle_bvh_tree.inl"
#include "triangle_primitive_mt.hpp"

namespace scene::shape::triangle {

template class bvh::Tree<bvh::Indexed_data<Shading_vertex_type>>;
// template class bvh::Tree<bvh::Indexed_data1>;
}  // namespace scene::shape::triangle
