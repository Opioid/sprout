#pragma once

#include "base/math/bounding/aabb.hpp"
#include <cstdint>
#include <vector>

namespace scene { namespace shape {

struct Vertex;

namespace triangle {

struct Index_triangle;

namespace bvh {

class Tree;

class Builder  {
public:

	void build(Tree& tree, const std::vector<Index_triangle>& triangles, const std::vector<Vertex>& vertices);

private:

	static math::AABB submesh_aabb(const std::vector<uint32_t>& primitive_indices, const std::vector<Index_triangle>& triangles, const std::vector<Vertex>& vertices);
};

}}}}


