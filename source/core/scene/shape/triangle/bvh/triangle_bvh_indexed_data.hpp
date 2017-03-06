#pragma once

#include "base/math/vector3.hpp"
#include "base/math/ray.hpp"
#include "base/math/simd/simd_vector.hpp"
#include <vector>

namespace scene { namespace shape {

struct Vertex;

namespace triangle { namespace bvh {

template<typename IV, typename SV>
class Indexed_data {

public:

	using Vertices = std::vector<Vertex>;

	Indexed_data();
	~Indexed_data();

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

	void allocate_triangles(uint32_t num_triangles, const Vertices& vertices);

	void add_triangle(uint32_t a, uint32_t b, uint32_t c,
					  uint32_t material_index, const Vertices& vertices);

	size_t num_bytes() const;

private:

	uint32_t num_triangles_;
	uint32_t current_triangle_;
	uint32_t num_vertices_;

	struct alignas(16) Index_triangle {
		Index_triangle(uint32_t a, uint32_t b, uint32_t c,
					   float bitangent_sign, uint32_t material_index);

		uint32_t a, b, c;
		uint32_t bts_material_index;

		static constexpr uint32_t Material_index_mask = 0x7FFFFFFF;
		static constexpr uint32_t BTS_mask = ~Material_index_mask;
	};

	Index_triangle* triangles_;
	IV*				intersection_vertices_;
	SV*				shading_vertices_;
};

template<typename V>
class Indexed_data_interleaved {

public:

	using Vertices = std::vector<Vertex>;

	Indexed_data_interleaved();
	~Indexed_data_interleaved();

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

	void allocate_triangles(uint32_t num_triangles, const Vertices& vertices);

	void add_triangle(uint32_t a, uint32_t b, uint32_t c,
					  uint32_t material_index, const Vertices& vertices);

	size_t num_bytes() const;

private:

	uint32_t num_triangles_;
	uint32_t current_triangle_;
	uint32_t num_vertices_;

	struct alignas(16) Index_triangle {
		Index_triangle(uint32_t a, uint32_t b, uint32_t c,
					   float bitangent_sign, uint32_t material_index);

		uint32_t a, b, c;
		uint32_t bts_material_index;

		static constexpr uint32_t Material_index_mask = 0x7FFFFFFF;
		static constexpr uint32_t BTS_mask = ~Material_index_mask;
	};

	Index_triangle* triangles_;
	V*				vertices_;
};

}}}}
