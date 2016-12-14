#include "testing_simd.hpp"
#include "base/chrono/chrono.hpp"
#include "base/math/vector.inl"
#include "base/random/generator.inl"
#include "base/math/simd/simd_math.inl"
#include "base/math/simd/simd_vector.inl"
#include "base/string/string.inl"
#include "base/math/print.hpp"
#include <iostream>

namespace testing { namespace simd {

inline float SSE_sqrt(float x) {
	_mm_store_ss(&x, _mm_sqrt_ss(_mm_load_ss(&x)));
	return x;
}

inline float SSE_rsqrt(float x) {
	_mm_store_ss(&x, _mm_rsqrt_ss(_mm_load_ss(&x)));
	return x;
}


inline float SSE_rsqrt_1N(float x) {
	float y = SSE_rsqrt(x);
	return y * (1.5f - 0.5f * x * y * y);
}

inline float simd_rsqrt(float x) {
  __m128 three = _mm_set1_ps(3.0f);
  __m128 half = _mm_set1_ps(0.5f);
  __m128 xs = _mm_load_ss(&x);
  __m128 res = _mm_rsqrt_ss(xs);
  __m128 muls = _mm_mul_ss(_mm_mul_ss(xs, res), res);
  _mm_store_ss(&x, _mm_mul_ss(_mm_mul_ss(half, res), _mm_sub_ss(three, muls)));
  return x;
}

void rsqrt() {
	std::cout << "testing::simd::rsqrt()" << std::endl;

	rnd::Generator rng(456, 90, 2123, 4598743);

	size_t num_values = 1024 * 1024 * (128 + 32);

	float* values = new float[num_values];

	for (size_t i = 0; i < num_values; ++i) {
		values[i] = 10.f * rng.random_float();
	}

	float a;

	{
		a = 0.f;
		auto start = std::chrono::high_resolution_clock::now();

		for (size_t i = 0; i < num_values; ++i) {
			float x = values[i];
			a += (1.f / std::sqrt(x));
			a = (1.f / std::sqrt(a));
		}

		auto duration = chrono::seconds_since(start);
		std::cout << a << " in " << string::to_string(duration) << " s" << std::endl;
	}

	{
		a = 0.f;
		auto start = std::chrono::high_resolution_clock::now();

		for (size_t i = 0; i < num_values; ++i) {
			float x = values[i];
			a += simd_rsqrt(x);
			a = simd_rsqrt(a);
		}

		auto duration = chrono::seconds_since(start);
		std::cout << a << " in " << string::to_string(duration) << " s" << std::endl;
	}

	{
		a = 0.f;
		auto start = std::chrono::high_resolution_clock::now();

		for (size_t i = 0; i < num_values; ++i) {
			float x = values[i];
			a += SSE_rsqrt(x);
			a = SSE_rsqrt(a);
		}

		auto duration = chrono::seconds_since(start);
		std::cout << a << " in " << string::to_string(duration) << " s" << std::endl;
	}

	{
		a = 0.f;
		auto start = std::chrono::high_resolution_clock::now();

		for (size_t i = 0; i < num_values; ++i) {
			float x = values[i];
			a += SSE_rsqrt_1N(x);
			a = SSE_rsqrt_1N(a);
		}

		auto duration = chrono::seconds_since(start);
		std::cout << a << " in " << string::to_string(duration) << " s" << std::endl;
	}

	{
		a = 0.f;
		auto start = std::chrono::high_resolution_clock::now();

		for (size_t i = 0; i < num_values; ++i) {
			float x = values[i];
			a += (1.f / std::sqrt(x));
			a = (1.f / std::sqrt(a));
		}

		auto duration = chrono::seconds_since(start);
		std::cout << a << " in " << string::to_string(duration) << " s" << std::endl;
	}

	delete [] values;
}


float3 simd_normalized_0(const float3& v) {
	float il = simd_rsqrt(math::dot(v, v));
	return il * v;
}

float3 simd_normalized_1(const float3& v) {
	math::simd::Vector sx = math::simd::load_float3(v);

	math::simd::Vector d = math::simd::dot3(sx, sx);

	math::simd::Vector il = math::simd::rsqrt(d);

	float3 result;
	math::simd::store_float3(result, _mm_mul_ps(il, sx));

	return result;
}

void normalize() {
	std::cout << "testing::simd::normalize()" << std::endl;

	rnd::Generator rng(456, 90, 2123, 4598743);

	size_t num_values = 1024 * 1024 * (128 + 0);

	float3* vectors = new float3[num_values];

	float3* results = new float3[num_values];

	for (size_t i = 0; i < num_values; ++i) {
		vectors[i] = float3(5.f - (10.f * rng.random_float()),
							5.f - (10.f * rng.random_float()),
							5.f - (10.f * rng.random_float()));
	}

	{
		std::cout << vectors[0] << " : " << math::normalized(vectors[0]) << std::endl;

		auto start = std::chrono::high_resolution_clock::now();

		for (size_t i = 0; i < num_values; ++i) {
			float3 x = vectors[i];
			float3 r = math::normalized(x);
			vectors[i] = r;
		}

		auto duration = chrono::seconds_since(start);
		std::cout << "Something in " << string::to_string(duration) << " s" << std::endl;
	}

	{
		std::cout << vectors[0] << " : " << simd_normalized_1(vectors[0]) << std::endl;

		auto start = std::chrono::high_resolution_clock::now();

		for (size_t i = 0; i < num_values; ++i) {
			float3 x = vectors[i];
			float3 r = simd_normalized_1(x);
			vectors[i] = r;
		}

		auto duration = chrono::seconds_since(start);
		std::cout << "Something in " << string::to_string(duration) << " s" << std::endl;
	}

	{
		std::cout << vectors[0] << " : " << simd_normalized_0(vectors[0]) << std::endl;

		auto start = std::chrono::high_resolution_clock::now();

		for (size_t i = 0; i < num_values; ++i) {
			float3 x = vectors[i];
			float3 r = simd_normalized_0(x);
			vectors[i] = r;
		}

		auto duration = chrono::seconds_since(start);
		std::cout << "Something in " << string::to_string(duration) << " s" << std::endl;
	}

	{
		std::cout << vectors[0] << " : " << math::normalized(vectors[0]) << std::endl;

		auto start = std::chrono::high_resolution_clock::now();

		for (size_t i = 0; i < num_values; ++i) {
			float3 x = vectors[i];
			float3 r = math::normalized(x);
			vectors[i] = r;
		}

		auto duration = chrono::seconds_since(start);
		std::cout << "Something in " << string::to_string(duration) << " s" << std::endl;
	}


	delete [] results;
	delete [] vectors;
}

}}
