#pragma once

//#include "triangle_bvh_builder_base.hpp"
#include "base/math/aabb.hpp"
#include "base/math/plane.hpp"
#include "base/math/vector3.hpp"
#include <vector>

namespace thread { class Pool; }

namespace scene { namespace shape {

struct Vertex;

namespace triangle {

struct Index_triangle;

namespace bvh {

template<typename Data> class Tree;
struct Node;

class Builder_SAH2 /*: private Builder_base*/  {

public:

	using Triangles = std::vector<Index_triangle>;
	using Vertices  = std::vector<Vertex>;

	Builder_SAH2(uint32_t num_slices, uint32_t sweep_threshold);

	template<typename Data>
	void build(Tree<Data>& tree, const Triangles& triangles, const Vertices& vertices,
			   uint32_t max_primitives, thread::Pool& thread_pool);

private:

	struct Reference {
		Reference() {};

		uint32_t primitive() const;

		void set_min_max_primitive(math::simd::FVector mi, math::simd::FVector ma,
								   uint32_t primitive);

		void clip_min(float d, uint8_t axis);
		void clip_max(float d, uint8_t axis);

		struct alignas(16) Vector {
			float v[3];
			uint32_t index;
		};

		Vector min;
		Vector max;
	};

	using References = std::vector<Reference>;

	class Split_candidate {

	public:

		Split_candidate(uint8_t split_axis, float3_p p, bool spatial);

		void evaluate(const References& references, float aabb_surface_area);

		void distribute(const References& references,
						References& references0, References& references1) const;

		float cost() const;

		bool behind(const float* point) const;

		uint8_t axis() const;
		bool spatial() const;

		const math::AABB& aabb_0() const;
		const math::AABB& aabb_1() const;

	private:

		math::AABB aabb_0_;
		math::AABB aabb_1_;

		uint32_t num_side_0_;
		uint32_t num_side_1_;

		float d_;

		float cost_;

		uint8_t axis_;

		bool spatial_;
	};

	struct Build_node {
		Build_node();
		~Build_node();

		math::AABB aabb;

		std::vector<uint32_t> primitives;

		uint32_t start_index;
		uint32_t end_index;

		uint8_t axis;

		Build_node* children[2];
	};

	void split(Build_node* node, References& references, const math::AABB& aabb,
			   uint32_t max_primitives, thread::Pool& thread_pool);

	Split_candidate splitting_plane(const References& references, const math::AABB& aabb,
									thread::Pool& thread_pool);

	template<typename Data>
	void serialize(Build_node* node, const Triangles& triangles,
				   const Vertices& vertices, Tree<Data>& tree);

	Node& new_node();

	uint32_t current_node_index() const;

	void assign(Build_node* node, const References& references);

	std::vector<Split_candidate> split_candidates_;

	uint32_t num_nodes_;
	uint32_t current_node_;

	uint32_t num_references_;

	Node* nodes_;

	const uint32_t num_slices_;
	const uint32_t sweep_threshold_;
};

}}}}
