#pragma once

#include "vertex_encoding.hpp"

namespace scene { namespace shape { namespace triangle {

//constexpr float F22 = static_cast<float>(1 << 22);
constexpr float FN = static_cast<float>((1 << 23) - 1);

inline void Norm23x2_Sign1x2::decode(math::packed_float3& v, float& s) const {
	const uint32_t a = (static_cast<uint32_t>(data[1]) & 0b01111111) << 16;
	const uint32_t x = a | static_cast<uint32_t>(data[0]);

	const uint32_t b = static_cast<uint32_t>(data[1]) >> 7;
	const uint32_t c = (static_cast<uint32_t>(data[2]) & 0b00111111'11111111) << 9;
	const uint32_t y = b | c;

	const uint32_t d = (static_cast<uint32_t>(data[2]) >> 14) & 0x1;
	const uint32_t e = (static_cast<uint32_t>(data[2]) >> 15) & 0x1;

//	const float fx = static_cast<float>(x) / F22 - 1.f;
//	const float fy = static_cast<float>(y) / F22 - 1.f;

	const float fx = (2.f * (static_cast<float>(x) / FN)) - 1.f;
	const float fy = (2.f * (static_cast<float>(y) / FN)) - 1.f;

	const float signs[2] = { 1.f, -1.f };

	const float sz = signs[d];
	const float fz = sz * std::sqrt(1.f - (fx * fx + fy * fy));

	v.x = fx;
	v.y = fy;
	v.z = fz;

	s = signs[e];
}

inline void Norm23x2_Sign1x2::encode(const math::packed_float3& v, float s) {
	const uint32_t x = static_cast<uint32_t>((v.x + 1.f) * (0.5f * FN));
	const uint32_t y = static_cast<uint32_t>((v.y + 1.f) * (0.5f * FN));

	const uint32_t zs = v.z < 0.f ? 1 : 0;
	const uint32_t is = s   < 0.f ? 1 : 0;

	data[0] = static_cast<uint16_t>(x);
	data[1] = static_cast<uint16_t>((x >> 16) | (y << 7));
	data[2] = static_cast<uint16_t>((y >> 9) | (zs << 14) | (is << 15));
}


}}}
