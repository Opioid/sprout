#pragma once

#include "base/math/vector.hpp"
#include "base/math/simd/simd_vector.hpp"
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

	bool intersect(uint32_t index, math::Oray& ray, math::float2& uv) const;

	bool intersect_p(uint32_t index, const math::Oray& ray) const;

	bool SU_CALLCONV intersect(uint32_t index,
							   math::simd::FVector origin, math::simd::FVector direction, float min_t, float max_t,
							   float& out_t, math::float2& uv);

	bool SU_CALLCONV intersect_p(uint32_t index,
								 math::simd::FVector origin, math::simd::FVector direction, float min_t, float max_t);

	void interpolate_data(uint32_t index, math::float2 uv,
						  math::vec3& n, math::vec3& t, math::float2& tc) const;

	math::float2 interpolate_uv(uint32_t index, math::float2 uv) const;

	float    bitangent_sign(uint32_t index) const;
	uint32_t material_index(uint32_t index) const;

	math::vec3 normal(uint32_t index) const;

	float area(uint32_t index) const;
	float area(uint32_t index, const math::vec3& scale) const;

//	void sample(uint32_t index, math::float2 r2, math::vec3& p, math::vec3& n, math::float2& tc) const;
	void sample(uint32_t index, math::float2 r2, math::vec3& p, math::float2& tc) const;
//	void sample(uint32_t index, math::float2 r2, math::vec3& p) const;

	void allocate_triangles(uint32_t num_triangles);

	void add_triangle(const Vertex& a, const Vertex& b, const Vertex& c, uint32_t material_index);

private:

	std::vector<Triangle> triangles_;
};

}}}}
