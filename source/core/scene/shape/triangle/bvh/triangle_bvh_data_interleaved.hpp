#pragma once

#include "base/math/vector.hpp"
#include <vector>

namespace scene { namespace shape {

struct Vertex;

namespace triangle { namespace bvh {

template<typename Triangle>
class Data_interleaved {
public:

	~Data_interleaved();

	uint32_t num_triangles() const;
	uint32_t current_triangle() const;

	bool intersect(uint32_t index, math::Oray& ray, float2& uv) const;

	bool intersect_p(uint32_t index, const math::Oray& ray) const;

	void interpolate_data(uint32_t index, float2 uv,
						  float3& n, float3& t, float2& tc) const;

	float2 interpolate_uv(uint32_t index, float2 uv) const;

	float    bitangent_sign(uint32_t index) const;
	uint32_t material_index(uint32_t index) const;

	float3 normal(uint32_t index) const;

	float area(uint32_t index) const;
	float area(uint32_t index, const float3& scale) const;

//	void sample(uint32_t index, float2 r2, float3& p, float3& n, float2& tc) const;
	void sample(uint32_t index, float2 r2, float3& p, float2& tc) const;
//	void sample(uint32_t index, float2 r2, float3& p) const;

	void allocate_triangles(uint32_t num_triangles, const std::vector<Vertex>& vertices);

	void add_triangle(uint32_t a, uint32_t b, uint32_t c, uint32_t material_index, const std::vector<Vertex>& vertices);

private:

	std::vector<Triangle> triangles_;
};

}}}}
