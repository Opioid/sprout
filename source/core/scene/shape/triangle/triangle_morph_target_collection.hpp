#pragma once

#include <vector>

namespace scene { namespace shape {

struct Vertex;

namespace triangle {

struct Index_triangle;

class Morph_target_collection {
public:

	std::vector<Index_triangle>& triangles();
	const std::vector<Index_triangle>& triangles() const;

	void add_swap_vertices(std::vector<Vertex>& vertices);

private:

	std::vector<Index_triangle> triangles_;


	std::vector<std::vector<Vertex>> morph_targets_;
};

}}}
