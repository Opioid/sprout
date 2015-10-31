#pragma once

#include "base/math/vector.hpp"
#include <vector>

namespace scene { namespace shape {

struct Vertex;

namespace triangle {

struct Triangle_MT;

namespace bvh {

class Data_MT {
public:

	inline ~Data_MT();

	inline uint32_t num_triangles() const;

	inline bool intersect(uint32_t index, math::Oray& ray, math::float2& uv) const;

	inline bool intersect_p(uint32_t index, const math::Oray& ray) const;

	inline void interpolate_data(uint32_t index, math::float2 uv,
								 math::float3& n, math::float3& t, math::float2& tc) const;

	inline math::float2 interpolate_uv(uint32_t index, math::float2 uv) const;

	inline float    bitangent_sign(uint32_t index) const;
	inline uint32_t material_index(uint32_t index) const;

	inline math::float3 normal(uint32_t index) const;

	inline float area(uint32_t index, const math::float3& scale) const;

	inline void sample(uint32_t index, math::float2 r2, math::float3& p, math::float3& n, math::float2& tc) const;
	inline void sample(uint32_t index, math::float2 r2, math::float3& p, math::float2& tc) const;
	inline void sample(uint32_t index, math::float2 r2, math::float3& p) const;

	inline void allocate_triangles(uint32_t num_triangles);

	inline void add_triangle(const Vertex& a, const Vertex& b, const Vertex& c, uint32_t material_index);

private:

	std::vector<Triangle_MT> triangles_;
};

}}}}
