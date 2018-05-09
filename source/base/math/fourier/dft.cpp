#include "dft.hpp"
#include "memory/align.hpp"
#include "math/sincos.hpp"
#include "math/math.hpp"
#include "math/simd_vector.inl"
#include "math/vector4.inl"
#include "thread/thread_pool.hpp"

namespace math {

int32_t dft_size(int32_t num) {
	return num / 2 + 1;
}

void dft_1d(float2* result, float const* source, int32_t num) {
	float const af = (-2.f * Pi) / static_cast<float>(num);

	int32_t const r  = num % 4;
	int32_t const m4 = num - r;

	const float4 zott = float4(0.f, 1.f, 2.f, 3.f);
	const Vector zv = simd::load_float4(zott.v);

	for (int32_t k = 0, len = num / 2; k <= len; ++k) {
		float const as = af * static_cast<float>(k);

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
			float const b = as * static_cast<float>(x);
			float sin_b;
			float cos_b;
			math::sincos(b, sin_b, cos_b);
			sum[0] += source[x] * cos_b;
			sum[1] += source[x] * sin_b;
		}

		result[k] = sum;
	}
}

void idft_1d(float* result, float2 const* source, int32_t num) {
	float const af = (-2.f * Pi) / static_cast<float>(num);

	for (int32_t x = 0; x < num; ++x) {
		float sum = source[0][0];

		float const a = af * static_cast<float>(x);

		int32_t const len = num / 2;
		for (int32_t k = 1; k < len; ++k) {
			float const b = a * static_cast<float>(k);

			float sin_b;
			float cos_b;
			math::sincos(b, sin_b, cos_b);

			sum += 2.f * (source[k][0] * cos_b + source[k][1] * sin_b);
		}

		float const b = a * static_cast<float>(len);

		float sin_b;
		float cos_b;
		math::sincos(b, sin_b, cos_b);

		sum += source[len][0] * cos_b + source[len][1] * sin_b;

		result[x] = sum;// / fn;
	}
}

// https://www.nayuki.io/page/how-to-implement-the-discrete-fourier-transform

void dft_2d(float2* result, float const* source, int32_t width, int32_t height,
			thread::Pool& pool) {
	int32_t row_size = dft_size(width);

	float2* tmp = memory::allocate_aligned<float2>(row_size * height);

	dft_2d(result, source, tmp, width, height, pool);

	memory::free_aligned(tmp);
}

void dft_2d(float2* result, float const* source, float2* tmp,
			int32_t width, int32_t height, thread::Pool& pool) {
	int32_t row_size = dft_size(width);

	pool.run_range([source, tmp, row_size, width](uint32_t /*id*/, int32_t begin, int32_t end) {
		for (int32_t y = begin; y < end; ++y) {
			dft_1d(tmp + y * row_size, source + y * width, width);
		}
	}, 0, height);

	pool.run_range([tmp, result, row_size, height](uint32_t /*id*/, int32_t begin, int32_t end) {
		float const af = (-2.f * Pi) / static_cast<float>(height);
		for (int32_t x = begin; x < end; ++x) {
			for (int32_t k = 0; k < height; ++k) {
				float2 sum(0.f);

				float const a = af * static_cast<float>(k);

				for (int32_t t = 0; t < height; ++t) {
					float const angle = a * static_cast<float>(t);

					float sin_a;
					float cos_a;
					math::sincos(angle, sin_a, cos_a);

					int32_t const g = t * row_size + x;
					sum[0] +=  tmp[g][0] * cos_a + tmp[g][1] * sin_a;
					sum[1] += -tmp[g][0] * sin_a + tmp[g][1] * cos_a;
				}

				int32_t const c = k * row_size + x;
				result[c] = sum;
			}
		}
	}, 0, row_size);
}

void idft_2d(float* result, float2 const* source, float2* tmp,
			 int32_t width, int32_t height, thread::Pool& pool) {
	int32_t row_size = dft_size(width);

	pool.run_range([tmp, source, row_size, height](uint32_t /*id*/, int32_t begin, int32_t end) {
		float const af = (2.f * Pi) / static_cast<float>(height);
		for (int32_t x = begin; x < end; ++x) {
			for (int32_t k = 0; k < height; ++k) {
				float2 sum(0.f);

				float const a = af * static_cast<float>(k);

				for (int32_t t = 0; t < height; ++t) {
					float const angle = a * static_cast<float>(t);

					float sin_a;
					float cos_a;
					math::sincos(angle, sin_a, cos_a);

					int32_t const g = t * row_size + x;
					sum[0] +=  source[g][0] * cos_a + source[g][1] * sin_a;
					sum[1] += -source[g][0] * sin_a + source[g][1] * cos_a;
				}

				int32_t const c = k * row_size + x;
				tmp[c] = sum;// / static_cast<float>(height);
			}
		}
	}, 0, row_size);

	pool.run_range([result, tmp, row_size, width](uint32_t /*id*/, int32_t begin, int32_t end) {
		for (int32_t y = begin; y < end; ++y) {
			idft_1d(result + y * width, tmp + y * row_size, width);
		}
	}, 0, height);
}

}
