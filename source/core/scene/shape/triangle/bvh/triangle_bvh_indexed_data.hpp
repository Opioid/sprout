#pragma once

#include "base/math/vector.hpp"
#include "base/math/simd/simd_vector.hpp"

namespace scene { namespace shape {

struct Vertex;

namespace triangle { namespace bvh {

template<typename Intersection_vertex, typename Shading_vertex>
class Indexed_data {

public:

	Indexed_data();
	~Indexed_data();

	uint32_t num_triangles() const;
	uint32_t current_triangle() const;

	bool intersect(uint32_t index, math::Ray& ray, float2& uv) const;

	bool intersect_p(uint32_t index, const math::Ray& ray) const;

	void interpolate_data(uint32_t index, float2 uv,
						  float3& n, float3& t, float2& tc) const;

	float2 interpolate_uv(uint32_t index, float2 uv) const;

	float    bitangent_sign(uint32_t index) const;
	uint32_t material_index(uint32_t index) const;

	float3 normal(uint32_t index) const;

	float area(uint32_t index) const;
	float area(uint32_t index, float3_p scale) const;

	void sample(uint32_t index, float2 r2, float3& p, float2& tc) const;

	void allocate_triangles(uint32_t num_triangles, const std::vector<Vertex>& vertices);

	void add_triangle(uint32_t a, uint32_t b, uint32_t c, uint32_t material_index,
					  const std::vector<Vertex>& vertices);

	size_t num_bytes() const;

private:

	uint32_t num_triangles_;
	uint32_t current_triangle_;
	uint32_t num_vertices_;

	struct Index_triangle {
		uint32_t a, b, c;
	};

	Index_triangle*      triangles_;
	Intersection_vertex* intersection_vertices_;
	Shading_vertex*		 shading_vertices_;
};

template<typename Intersection_vertex, typename Shading_vertex>
class Indexed_data1 {

public:

	Indexed_data1();
	~Indexed_data1();

	uint32_t num_triangles() const;
	uint32_t current_triangle() const;

	bool intersect(uint32_t index, math::Ray& ray, float2& uv) const;

	bool intersect_p(uint32_t index, const math::Ray& ray) const;

	bool intersect_p(math::simd::FVector origin,
					 math::simd::FVector direction,
					 math::simd::FVector min_t,
					 math::simd::FVector max_t,
					 uint32_t index) const;

	void interpolate_data(uint32_t index, float2 uv,
						  float3& n, float3& t, float2& tc) const;

	float2 interpolate_uv(uint32_t index, float2 uv) const;

	float    bitangent_sign(uint32_t index) const;
	uint32_t material_index(uint32_t index) const;

	float3 normal(uint32_t index) const;

	float area(uint32_t index) const;
	float area(uint32_t index, float3_p scale) const;

	void sample(uint32_t index, float2 r2, float3& p, float2& tc) const;

	void allocate_triangles(uint32_t num_triangles, const std::vector<Vertex>& vertices);

	void add_triangle(uint32_t a, uint32_t b, uint32_t c, uint32_t material_index,
					  const std::vector<Vertex>& vertices);

	size_t num_bytes() const;

private:

	uint32_t num_triangles_;
	uint32_t current_triangle_;
	uint32_t num_vertices_;

	struct alignas(16) Index_triangle {
		Index_triangle(uint32_t a, uint32_t b, uint32_t c,
					   uint32_t material_index, float bitangent_sign);

		uint32_t a, b, c;
		uint32_t bts_material_index;

		static constexpr uint32_t Material_index_mask = 0x7fffffff;
		static constexpr uint32_t BTS_mask = ~Material_index_mask;
	};

	Index_triangle*      triangles_;
	Intersection_vertex* intersection_vertices_;
	Shading_vertex*		 shading_vertices_;
};


}}}}
