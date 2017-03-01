#pragma once

#include "base/math/vector.hpp"

namespace scene { namespace shape { namespace triangle {

struct Norm23x2_Sign1x2 {
	void decode(math::packed_float3& v, float& s) const;
	void encode(const math::packed_float3& v, float s);

	uint16_t data[3];
};

struct Compressed_vertex {
	math::packed_float3 p;
	Norm23x2_Sign1x2 n;
	Norm23x2_Sign1x2 t;
	float2 uv;
};

}}}
