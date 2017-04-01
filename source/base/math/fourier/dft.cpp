#include "dft.hpp"
#include "memory/align.hpp"
#include "math/sincos.hpp"
#include "math/math.hpp"
#include "math/vector2.inl"
#include "math/vector.inl"
#include "thread/thread_pool.hpp"

namespace math {

int32_t dft_size(int32_t num) {
	return num / 2 + 1;
}

void dft_1d(float2* result, const float* source, int32_t num) {
	const float af = (-2.f * Pi) / static_cast<float>(num);

	for (int32_t k = 0, len = num / 2; k <= len; ++k) {
		float2 sum(0.f);

		const float a = af * static_cast<float>(k);

		for (int32_t x = 0; x < num; ++x) {
			const float b = a * static_cast<float>(x);
			float sin_b;
			float cos_b;
			math::sincos(b, sin_b, cos_b);
			sum[0] += source[x] * cos_b;
			sum[1] += source[x] * sin_b;
		}

		result[k] = sum;

		// normalization
//		result[k].x *= (k == 0 || k == len) ? 1.f / fn : 2.f / fn;
//		result[k].y *= 2.f / fn;
	}
}

void dft_1d_v(float2* result, const float* source, int32_t num) {
	const float af = (-2.f * Pi) / static_cast<float>(num);

	for (int32_t k = 0, len = num / 2; k <= len; ++k) {
		Vector sum_x = simd::Zero;
		Vector sum_y = simd::Zero;

		const Vector a = load_float(af * static_cast<float>(k));

		for (int32_t x = 0; x < num; x += 4) {
			float xf = static_cast<float>(x);
			float4 xs = float4(xf, xf + 1.f, xf + 2.f, xf + 3.f);
			Vector b = math::mul4(a, load_float4(xs));
			Vector sin_b;
			Vector cos_b;
			math::sincos(b, sin_b, cos_b);
			Vector vx = load_unaligned_float4(&source[x]);
			sum_x = math::add4(sum_x, math::mul4(vx, cos_b));
			sum_y = math::add4(sum_y, math::mul4(vx, sin_b));
		}

		float4 sx;
		store_float4(sx, sum_x);
		float4 sy;
		store_float4(sy, sum_y);

		result[k] = float2(sx[0] + sx[1] + sx[2] + sx[3],
						   sy[0] + sy[1] + sy[2] + sy[3]);
	}
}

void idft_1d(float* result, const float2* source, int32_t num) {
	float fn = static_cast<float>(num);

	for (int32_t x = 0; x < num; ++x) {
		float sum = source[0][0];

		float a = -2.f * Pi * static_cast<float>(x) / fn;

		for (int32_t k = 1, len = num / 2; k <= len; ++k) {
			float b = a * static_cast<float>(k);

			sum += source[k][0] * std::cos(b) + source[k][1] * std::sin(b);
		}

		result[x] = sum;
	}
}

// https://www.nayuki.io/page/how-to-implement-the-discrete-fourier-transform

void dft_2d(float2* result, const float* source, int32_t width, int32_t height) {
	int32_t row_size = dft_size(width);

	float2* tmp = memory::allocate_aligned<float2>(height * row_size);

	for (int32_t y = 0; y < height; ++y) {
		dft_1d_v(tmp + y * row_size, source + y * width, width);
	}

	const float af = (2.f * Pi) / static_cast<float>(height);

	for (int32_t x = 0; x < row_size; ++x) {
		for (int32_t k = 0; k < height; ++k) {
			float2 sum(0.f);

			const float a = af * static_cast<float>(k);

			for (int32_t t = 0; t < height; ++t) {
				float angle = a * static_cast<float>(t);

//				float cos_a = std::cos(angle);
//				float sin_a = std::sin(angle);
				float sin_a;
				float cos_a;
				math::sincos(angle, sin_a, cos_a);

				int32_t g = t * row_size + x;

				sum[0] +=  tmp[g][0] * cos_a + tmp[g][1] * sin_a;
				sum[1] += -tmp[g][0] * sin_a + tmp[g][1] * cos_a;
			}

			int32_t c = k * row_size + x;
			result[c] = sum;
		}
	}

	memory::free_aligned(tmp);
}

void dft_2d(float2* result, const float* source, int32_t width, int32_t height,
			thread::Pool& pool) {
	int32_t row_size = dft_size(width);

	float2* tmp = new float2[height * row_size];

	pool.run_range([source, tmp, row_size, width](uint32_t /*id*/, int32_t begin, int32_t end) {
		for (int32_t y = begin; y < end; ++y) {
			dft_1d(tmp + y * row_size, source + y * width, width);
		}
	}, 0, height);

	pool.run_range([tmp, result, row_size, height](uint32_t /*id*/, int32_t begin, int32_t end) {
		float fn = static_cast<float>(height);
		for (int32_t x = begin; x < end; ++x) {
			for (int32_t k = 0; k < height; ++k) {
				float2 sum(0.f);

				float a = 2.f * Pi * static_cast<float>(k) / fn;

				for (int32_t t = 0; t < height; ++t) {
					int32_t g = t * row_size + x;

					float angle = a * static_cast<float>(t);

					float cos_a = std::cos(angle);
					float sin_a = std::sin(angle);

					sum[0] +=  tmp[g][0] * cos_a + tmp[g][1] * sin_a;
					sum[1] += -tmp[g][0] * sin_a + tmp[g][1] * cos_a;
				}

				int32_t c = k * row_size + x;
				result[c] = sum;
			}
		}
	}, 0, row_size);

	delete[] tmp;
}

}
