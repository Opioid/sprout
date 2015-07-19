#pragma once

#include "sampler_2d_linear.hpp"
#include "image/texture/texture_2d.hpp"
#include <algorithm>

namespace image { namespace texture { namespace sampler {

inline float bilinear_1(float c00, float c01, float c10, float c11, float s, float t);

inline math::float2 bilinear_2(math::float2 c00, math::float2 c01, math::float2 c10, math::float2 c11, float s, float t);

template<typename T>
T bilinear(const T& c00, const T& c01, const T& c10, const T& c11, float s, float t);

template<typename Address_mode>
float Sampler_2D_linear<Address_mode>::sample_1(const Texture_2D& texture, math::float2 uv) const {
	auto d  = texture.dimensions();
	auto df = texture.dimensions_float();

	uv = address_mode_.f(uv);

	float u = uv.x * df.x - 0.5f;
	float v = uv.y * df.y - 0.5f;

	float fu = std::floor(u);
	float fv = std::floor(v);

	int x = static_cast<int>(fu);
	int y = static_cast<int>(fv);

	int x1 = std::min(x + 1, static_cast<int>(d.x) - 1);
	int y1 = std::min(y + 1, static_cast<int>(d.y) - 1);

	x = std::max(x, 0);
	y = std::max(y, 0);

	float c00 = texture.at_1(x,  y);
	float c01 = texture.at_1(x,  y1);
	float c10 = texture.at_1(x1, y);
	float c11 = texture.at_1(x1, y1);

	float s = u - fu;
	float t = v - fv;

	return bilinear_1(c00, c01, c10, c11, s, t);
}

template<typename Address_mode>
math::float2 Sampler_2D_linear<Address_mode>::sample_2(const Texture_2D& texture, math::float2 uv) const {
	auto d  = texture.dimensions();
	auto df = texture.dimensions_float();

	uv = address_mode_.f(uv);

	float u = uv.x * df.x - 0.5f;
	float v = uv.y * df.y - 0.5f;

	float fu = std::floor(u);
	float fv = std::floor(v);

	int x = static_cast<int>(fu);
	int y = static_cast<int>(fv);

	int x1 = std::min(x + 1, static_cast<int>(d.x) - 1);
	int y1 = std::min(y + 1, static_cast<int>(d.y) - 1);

	x = std::max(x, 0);
	y = std::max(y, 0);

	math::float2 c00 = texture.at_2(x,  y);
	math::float2 c01 = texture.at_2(x,  y1);
	math::float2 c10 = texture.at_2(x1, y);
	math::float2 c11 = texture.at_2(x1, y1);

	float s = u - fu;
	float t = v - fv;

	return bilinear_2(c00, c01, c10, c11, s, t);
}

template<typename Address_mode>
math::float3 Sampler_2D_linear<Address_mode>::sample_3(const Texture_2D& texture, math::float2 uv) const {
	auto d  = texture.dimensions();
	auto df = texture.dimensions_float();

	uv = address_mode_.f(uv);

	uv = math::float2::identity;

	float u = uv.x * df.x - 0.5f;
	float v = uv.y * df.y - 0.5f;

	float fu = std::floor(u);
	float fv = std::floor(v);

	int x = static_cast<int>(fu);
	int y = static_cast<int>(fv);

	int x1 = std::min(x + 1, static_cast<int>(d.x) - 1);
	int y1 = std::min(y + 1, static_cast<int>(d.y) - 1);

	x = std::max(x, 0);
	y = std::max(y, 0);

	math::float3 c00 = texture.at_3(x,  y);
	math::float3 c01 = texture.at_3(x,  y1);
	math::float3 c10 = texture.at_3(x1, y);
	math::float3 c11 = texture.at_3(x1, y1);

	float s = u - fu;
	float t = v - fv;

	return bilinear(c00, c01, c10, c11, s, t);
}

template<typename Address_mode>
math::float2 Sampler_2D_linear<Address_mode>::address(math::float2 uv) const {
	return address_mode_.f(uv);
}

inline float bilinear_1(float c00, float c01, float c10, float c11, float s, float t) {
	float _s = 1.f - s;
	float _t = 1.f - t;

	return _s * (_t * c00 + t * c01) + s * (_t * c10 + t * c11);
}

inline math::float2 bilinear_2(math::float2 c00, math::float2 c01, math::float2 c10, math::float2 c11, float s, float t) {
	float _s = 1.f - s;
	float _t = 1.f - t;

	return _s * (_t * c00 + t * c01) + s * (_t * c10 + t * c11);
}

template<typename T>
T bilinear(const T& c00, const T& c01, const T& c10, const T& c11, float s, float t) {
	float _s = 1.f - s;
	float _t = 1.f - t;

	return _s * (_t * c00 + t * c01) + s * (_t * c10 + t * c11);
}

}}}

