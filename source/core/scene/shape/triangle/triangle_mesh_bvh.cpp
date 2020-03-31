#include "triangle_mesh_bvh.hpp"
#include "bvh/triangle_bvh_tree.inl"
#include "triangle_primitive_mt.hpp"
#include "bvh/triangle_bvh_indexed_data.inl"

namespace scene::shape::triangle {

template class bvh::Tree<bvh::Indexed_data<Shading_vertex_type>>;
}
