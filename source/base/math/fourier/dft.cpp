#include "dft.hpp"
#include "memory/align.hpp"
#include "math/sincos.hpp"
#include "math/math.hpp"
#include "math/vector.inl"
#include "math/vector2.inl"
#include "thread/thread_pool.hpp"

namespace math {

int32_t dft_size(int32_t num) {
	return num / 2 + 1;
}

void dft_1d(float2* result, const float* source, int32_t num) {
	const float af = (-2.f * Pi) / static_cast<float>(num);

	const int32_t r  = num % 4;
	const int32_t m4 = num - r;

	const float4 zott = float4(0.f, 1.f, 2.f, 3.f);
	const Vector zv = simd::load_float4(zott.v);

	for (int32_t k = 0, len = num / 2; k <= len; ++k) {
		const float as = af * static_cast<float>(k);

		// Use SSE to work on blocks of 4
		const Vector a = simd::set_float4(as);

		Vector sum_x = simd::Zero;
		Vector sum_y = simd::Zero;

		for (int32_t x = 0; x < m4; x += 4) {
			const Vector xf = simd::set_float4(static_cast<float>(x));
			const Vector xv = math::add(xf, zv);
			const Vector b = math::mul(a, xv);
			Vector sin_b;
			Vector cos_b;
			math::sincos(b, sin_b, cos_b);
			const Vector sv = simd::load_unaligned_float4(&source[x]);
			sum_x = math::add(sum_x, mul(sv, cos_b));
			sum_y = math::add(sum_y, mul(sv, sin_b));
		}

		float2 sum(horizontal_sum(sum_x), horizontal_sum(sum_y));

		// Use scalar operations to handle the rest
		for (int32_t x = m4; x < num; ++x) {
			const float b = as * static_cast<float>(x);
			float sin_b;
			float cos_b;
			math::sincos(b, sin_b, cos_b);
			sum[0] += source[x] * cos_b;
			sum[1] += source[x] * sin_b;
		}

		result[k] = sum;
	}
}

void idft_1d(float* result, const float2* source, int32_t num) {
	float fn = static_cast<float>(num);

	for (int32_t x = 0; x < num; ++x) {
		float sum = source[0][0];

		float a = -2.f * Pi * static_cast<float>(x) / fn;

		const int32_t len = num / 2;
		for (int32_t k = 1; k < len; ++k) {
			float b = a * static_cast<float>(k);

			sum += 2.f * (source[k][0] * std::cos(b) + source[k][1] * std::sin(b));
		}


		float b = a * static_cast<float>(len);
		sum += source[len][0] * std::cos(b) + source[len][1] * std::sin(b);

		result[x] = sum / fn;
	}

}

// https://www.nayuki.io/page/how-to-implement-the-discrete-fourier-transform

void dft_2d(float2* result, const float* source, int32_t width, int32_t height) {
	int32_t row_size = dft_size(width);

	float2* tmp = memory::allocate_aligned<float2>(height * row_size);

	for (int32_t y = 0; y < height; ++y) {
		dft_1d(tmp + y * row_size, source + y * width, width);
	}

	const float af = (2.f * Pi) / static_cast<float>(height);

	for (int32_t x = 0; x < row_size; ++x) {
		for (int32_t k = 0; k < height; ++k) {
			float2 sum(0.f);

			const float a = af * static_cast<float>(k);

			for (int32_t t = 0; t < height; ++t) {
				const float angle = a * static_cast<float>(t);

				float sin_a;
				float cos_a;
				math::sincos(angle, sin_a, cos_a);

				const int32_t g = t * row_size + x;
				sum[0] +=  tmp[g][0] * cos_a + tmp[g][1] * sin_a;
				sum[1] += -tmp[g][0] * sin_a + tmp[g][1] * cos_a;
			}

			const int32_t c = k * row_size + x;
			result[c] = sum;
		}
	}

	memory::free_aligned(tmp);
}

void dft_2d(float2* result, const float* source, int32_t width, int32_t height,
			thread::Pool& pool) {
	int32_t row_size = dft_size(width);

	float2* tmp = memory::allocate_aligned<float2>(height * row_size);

	pool.run_range([source, tmp, row_size, width](uint32_t /*id*/, int32_t begin, int32_t end) {
		for (int32_t y = begin; y < end; ++y) {
			dft_1d(tmp + y * row_size, source + y * width, width);
		}
	}, 0, height);

	pool.run_range([tmp, result, row_size, height](uint32_t /*id*/, int32_t begin, int32_t end) {
		const float af = (2.f * Pi) / static_cast<float>(height);
		for (int32_t x = begin; x < end; ++x) {
			for (int32_t k = 0; k < height; ++k) {
				float2 sum(0.f);

				const float a = af * static_cast<float>(k);

				for (int32_t t = 0; t < height; ++t) {
					const float angle = a * static_cast<float>(t);

					float sin_a;
					float cos_a;
					math::sincos(angle, sin_a, cos_a);

					const int32_t g = t * row_size + x;
					sum[0] +=  tmp[g][0] * cos_a + tmp[g][1] * sin_a;
					sum[1] += -tmp[g][0] * sin_a + tmp[g][1] * cos_a;
				}

				const int32_t c = k * row_size + x;
				result[c] = sum;
			}
		}
	}, 0, row_size);

	memory::free_aligned(tmp);
}

void idft_2d(float* result, const float2* source, int32_t width, int32_t height) {

}

}
