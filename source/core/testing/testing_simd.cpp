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

void rsqrt() {
	std::cout << "testing::simd::rsqrt()" << std::endl;

	rnd::Generator rng(456, 90, 2123, 4598743);

	size_t num_values = 1024 * 1024 * (128 + 32);

	float* values = new float[num_values];

	for (size_t i = 0; i < num_values; ++i) {
		values[i] = 10.f * rng.random_float();
	}

	{
		float a = 0.f;
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
		float a = 0.f;
		auto start = std::chrono::high_resolution_clock::now();

		for (size_t i = 0; i < num_values; ++i) {
			float x = values[i];
			a += math::simd::rsqrt(x);
			a = math::simd::rsqrt(a);
		}

		auto duration = chrono::seconds_since(start);
		std::cout << a << " in " << string::to_string(duration) << " s" << std::endl;
	}

	{
		float a = 0.f;
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
		float a = 0.f;
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

inline float SSE_rcp(float x) {
	_mm_store_ss(&x, _mm_rcp_ss(_mm_load_ss(&x)));
	return x;
}

inline float SSE_rcp_1N(float x) {
	__m128 sx = _mm_load_ss(&x);
	__m128 res = _mm_rcp_ss(sx);
	__m128 muls = _mm_mul_ss(sx, _mm_mul_ss(res, res));
	_mm_store_ss(&x, _mm_sub_ss(_mm_add_ss(res, res), muls));
	return x;
}

void rcp() {
	std::cout << "testing::simd::rcp()" << std::endl;

	rnd::Generator rng(456, 90, 2123, 4598743);

	size_t num_values = 1024 * 1024 * (128 + 8);

	float* values = new float[num_values];

	for (size_t i = 0; i < num_values; ++i) {
		values[i] = 10.f * rng.random_float();
	}

	{
		float a = 0.f;
		auto start = std::chrono::high_resolution_clock::now();

		for (size_t i = 0; i < num_values; ++i) {
			float x = values[i];
			a += (1.f / x);
			a = (1.f / a);
		}

		auto duration = chrono::seconds_since(start);
		std::cout << a << " in " << string::to_string(duration) << " s" << std::endl;
	}

	{
		float a = 0.f;
		auto start = std::chrono::high_resolution_clock::now();

		for (size_t i = 0; i < num_values; ++i) {
			float x = values[i];
			a += SSE_rcp(x);
			a = SSE_rcp(a);
		}

		auto duration = chrono::seconds_since(start);
		std::cout << a << " in " << string::to_string(duration) << " s" << std::endl;
	}

	{
		float a = 0.f;
		auto start = std::chrono::high_resolution_clock::now();

		for (size_t i = 0; i < num_values; ++i) {
			float x = values[i];
			a += SSE_rcp_1N(x);
			a = SSE_rcp_1N(a);
		}

		auto duration = chrono::seconds_since(start);
		std::cout << a << " in " << string::to_string(duration) << " s" << std::endl;
	}

	{
		float a = 0.f;
		auto start = std::chrono::high_resolution_clock::now();

		for (size_t i = 0; i < num_values; ++i) {
			float x = values[i];
			a += (1.f / x);
			a = (1.f / a);
		}

		auto duration = chrono::seconds_since(start);
		std::cout << a << " in " << string::to_string(duration) << " s" << std::endl;
	}

	delete [] values;
}

inline float3 simd_normalized_0(const float3& v) {
	float il = math::simd::rsqrt(math::dot(v, v));
	return il * v;
}

inline float3 simd_normalized_1(const float3& v) {
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

	for (size_t i = 0; i < num_values; ++i) {
		vectors[i] = float3(5.f - (10.f * rng.random_float()),
							5.f - (10.f * rng.random_float()),
							5.f - (10.f * rng.random_float()));
	}

	{
		std::cout << vectors[0] << " : " << math::normalized(vectors[0]) << std::endl;

		auto start = std::chrono::high_resolution_clock::now();

		float3 result(0.f);

		for (size_t i = 0; i < num_values; ++i) {
			float3 x = vectors[i];
			float3 r = math::normalized(x);
			result = math::normalized(r + result);
		}

		auto duration = chrono::seconds_since(start);
		std::cout << result << " in " << string::to_string(duration) << " s" << std::endl;
	}

	{
		std::cout << vectors[0] << " : " << simd_normalized_0(vectors[0]) << std::endl;

		auto start = std::chrono::high_resolution_clock::now();

		float3 result(0.f);

		for (size_t i = 0; i < num_values; ++i) {
			float3 x = vectors[i];
			float3 r = simd_normalized_0(x);
			result = simd_normalized_0(r + result);
		}

		auto duration = chrono::seconds_since(start);
		std::cout << result << " in " << string::to_string(duration) << " s" << std::endl;
	}

	{
		std::cout << vectors[0] << " : " << simd_normalized_1(vectors[0]) << std::endl;

		auto start = std::chrono::high_resolution_clock::now();

		float3 result(0.f);

		for (size_t i = 0; i < num_values; ++i) {
			float3 x = vectors[i];
			float3 r = simd_normalized_1(x);
			result = simd_normalized_1(r + result);
		}

		auto duration = chrono::seconds_since(start);
		std::cout << result << " in " << string::to_string(duration) << " s" << std::endl;
	}

	{
		std::cout << vectors[0] << " : " << math::normalized(vectors[0]) << std::endl;

		auto start = std::chrono::high_resolution_clock::now();

		float3 result(0.f);

		for (size_t i = 0; i < num_values; ++i) {
			float3 x = vectors[i];
			float3 r = math::normalized(x);
			result = math::normalized(r + result);
		}

		auto duration = chrono::seconds_since(start);
		std::cout << result << " in " << string::to_string(duration) << " s" << std::endl;
	}

	delete [] vectors;
}


//inline float SSE_rcp_1N(float x) {
//	__m128 sx = _mm_load_ss(&x);
//	__m128 res = _mm_rcp_ss(sx);
//	__m128 muls = _mm_mul_ss(sx, _mm_mul_ss(res, res));
//	_mm_store_ss(&x, _mm_sub_ss(_mm_add_ss(res, res), muls));
//	return x;
//}

inline float3 simd_reciprocal(const float3& v) {
	math::simd::Vector sx = math::simd::load_float3(v);

	__m128 rcp = _mm_rcp_ps(sx);
	__m128 mul = _mm_mul_ps(sx, _mm_mul_ps(rcp, rcp));

	float3 result;
	math::simd::store_float3(result, _mm_sub_ps(_mm_add_ps(rcp, rcp), mul));
	return result;
}

inline void simd_reciprocal(float3& result, const float3& v) {
	math::simd::Vector sx = math::simd::load_float3(v);

	__m128 rcp = _mm_rcp_ps(sx);
	__m128 mul = _mm_mul_ps(sx, _mm_mul_ps(rcp, rcp));

	math::simd::store_float3(result, _mm_sub_ps(_mm_add_ps(rcp, rcp), mul));
}

void reciprocal() {
	std::cout << "testing::simd::reciprocal()" << std::endl;

	rnd::Generator rng(456, 90, 2123, 4598743);

	size_t num_values = 1024 * 1024 * (9);

	float3* vectors = new float3[num_values];

	for (size_t i = 0; i < num_values; ++i) {
		vectors[i] = float3(5.f - (10.f * rng.random_float()),
							5.f - (10.f * rng.random_float()),
							5.f - (10.f * rng.random_float()));
	}

	{
		std::cout << vectors[0] << " : " << math::reciprocal(vectors[0]) << std::endl;

		auto start = std::chrono::high_resolution_clock::now();

		float3 result(0.f);

		for (size_t i = 0; i < num_values; ++i) {
			float3 x = vectors[i];
			float3 r = math::reciprocal(x);
			result = math::reciprocal(r + result);
		}

		auto duration = chrono::seconds_since(start);
		std::cout << result << " in " << string::to_string(duration) << " s" << std::endl;
	}

	{
		std::cout << vectors[0] << " : " << simd_reciprocal(vectors[0]) << std::endl;

		auto start = std::chrono::high_resolution_clock::now();

		float3 result(0.f);

		for (size_t i = 0; i < num_values; ++i) {
			float3 x = vectors[i];
			float3 r = simd_reciprocal(x);
			result = simd_reciprocal(r + result);
		}

		auto duration = chrono::seconds_since(start);
		std::cout << result << " in " << string::to_string(duration) << " s" << std::endl;
	}

	{
		std::cout << vectors[0] << " : " << simd_normalized_1(vectors[0]) << std::endl;

		auto start = std::chrono::high_resolution_clock::now();

		float3 result(0.f);

		for (size_t i = 0; i < num_values; ++i) {
			float3 x = vectors[i];
			float3 r;
			simd_reciprocal(r, x);
			simd_reciprocal(result, r + result);
		}

		auto duration = chrono::seconds_since(start);
		std::cout << result << " in " << string::to_string(duration) << " s" << std::endl;
	}

	{
		std::cout << vectors[0] << " : " << math::normalized(vectors[0]) << std::endl;

		auto start = std::chrono::high_resolution_clock::now();

		float3 result(0.f);

		for (size_t i = 0; i < num_values; ++i) {
			float3 x = vectors[i];
			float3 r = math::reciprocal(x);
			result = math::reciprocal(r + result);
		}

		auto duration = chrono::seconds_since(start);
		std::cout << result << " in " << string::to_string(duration) << " s" << std::endl;
	}

	delete [] vectors;
}

inline float dotly(float3 a, float3 b) {
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline float simd_dot_0(const float3& a, const float3& b) {
	math::simd::Vector sa = math::simd::load_float3(a);
	math::simd::Vector sb = math::simd::load_float3(b);

	math::simd::Vector d = math::simd::dot3(sa, sb);

	float3 result;
	math::simd::store_float3(result, d);

	return result.x;
}

inline float simd_dot_1(const float3 a, const float3 b) {
	math::simd::Vector sa = math::simd::load_float3(a);
	math::simd::Vector sb = math::simd::load_float3(b);

	// Perform the dot product
	math::simd::Vector vDot = _mm_mul_ps(sa, sb);
	// x=Dot.vector4_f32[1], y=Dot.vector4_f32[2]
	math::simd::Vector vTemp = SU_PERMUTE_PS(vDot, _MM_SHUFFLE(2, 1, 2, 1));
	// Result.vector4_f32[0] = x+y
	vDot = _mm_add_ss(vDot, vTemp);
	// x=Dot.vector4_f32[2]
	vTemp = SU_PERMUTE_PS(vTemp, _MM_SHUFFLE(1, 1, 1, 1));
	// Result.vector4_f32[0] = (x+y)+z
	vDot = _mm_add_ss(vDot, vTemp);
	// Splat x

	float x;
	_mm_store_ss(&x, vDot);
	return x;

//	return SU_PERMUTE_PS(vDot, _MM_SHUFFLE(0, 0, 0, 0));
}

void dot() {
	std::cout << "testing::simd::dot()" << std::endl;

	rnd::Generator rng(456, 90, 2123, 4598743);

	size_t num_values = 1024 * 1024 * (128 + 16);

	float3* vectors = new float3[num_values];

	for (size_t i = 0; i < num_values; ++i) {
		vectors[i] = float3(5.f - (10.f * rng.random_float()),
							5.f - (10.f * rng.random_float()),
							5.f - (10.f * rng.random_float()));
	}

	{
		std::cout << vectors[0] << " : " << math::dot(vectors[0], vectors[1]) << std::endl;

		auto start = std::chrono::high_resolution_clock::now();

		float result = 0.f;

		for (size_t i = 0, len = num_values - 1; i < len; ++i) {
			float3 a = vectors[i];
			float3 b = vectors[i + 1];
			float r = math::dot(a, b);
			result += r;
		}

		auto duration = chrono::seconds_since(start);
		std::cout << result << " in " << string::to_string(duration) << " s" << std::endl;
	}

	{
		std::cout << vectors[0] << " : " << simd_dot_0(vectors[0], vectors[1]) << std::endl;

		auto start = std::chrono::high_resolution_clock::now();

		float result = 0.f;

		for (size_t i = 0, len = num_values - 1; i < len; ++i) {
			float3 a = vectors[i];
			float3 b = vectors[i + 1];
			float r = simd_dot_0(a, b);
			result += r;
		}

		auto duration = chrono::seconds_since(start);
		std::cout << result << " in " << string::to_string(duration) << " s" << std::endl;
	}

	{
		std::cout << vectors[0] << " : " << simd_dot_1(vectors[0], vectors[1]) << std::endl;

		auto start = std::chrono::high_resolution_clock::now();

		float result = 0.f;

		for (size_t i = 0, len = num_values - 1; i < len; ++i) {
			float3 a = vectors[i];
			float3 b = vectors[i + 1];
			float r = simd_dot_1(a, b);
			result += r;
		}

		auto duration = chrono::seconds_since(start);
		std::cout << result << " in " << string::to_string(duration) << " s" << std::endl;
	}

	{
		std::cout << vectors[0] << " : " << math::dot(vectors[0], vectors[1]) << std::endl;

		auto start = std::chrono::high_resolution_clock::now();

		float result = 0.f;

		for (size_t i = 0, len = num_values - 1; i < len; ++i) {
			float3 a = vectors[i];
			float3 b = vectors[i + 1];
			float r = dotly(a, b);
			result += r;
		}

		auto duration = chrono::seconds_since(start);
		std::cout << result << " in " << string::to_string(duration) << " s" << std::endl;
	}
}

}}
