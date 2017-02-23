#include "dft.hpp"
#include "math/math.hpp"
#include "math/vector.inl"
#include "thread/thread_pool.hpp"

namespace math {

int32_t dft_size(int32_t num) {
	return num / 2 + 1;
}

void dft_1d(float2* result, const float* source, int32_t num) {
	float fn = static_cast<float>(num);

	bool even = true;

	for (int32_t k = 0, len = num / 2; k <= len; ++k, even = !even) {
		float2 sum(0.f);

		float a = -2.f * Pi / fn * static_cast<float>(k);

		if (even) {
			for (int32_t x = 0; x < num; ++x) {
				float b = a * static_cast<float>(x);
				sum.x += source[x] * std::cos(b);
		//		sum.y += source[x] * std::sin(b);
			}
		} else {
			for (int32_t x = 0; x < num; ++x) {
				float b = a * static_cast<float>(x);
				sum.x += source[x] * std::cos(b);
				sum.y += source[x] * std::sin(b);
			}
		}

		result[k] = sum;

		// normalization
//		result[k].x *= (k == 0 || k == len) ? 1.f / fn : 2.f / fn;
//		result[k].y *= 2.f / fn;
	}
}

void idft_1d(float* result, const float2* source, int32_t num) {
	float fn = static_cast<float>(num);

	for (int32_t x = 0; x < num; ++x) {
		float sum = source[0].x;

		float a = -2.f * Pi * static_cast<float>(x) / fn;

		for (int32_t k = 1, len = num / 2; k <= len; ++k) {
			float b = a * static_cast<float>(k);

			sum += source[k].x * std::cos(b) + source[k].y * std::sin(b);
		}

		result[x] = sum;
	}
}

// https://www.nayuki.io/page/how-to-implement-the-discrete-fourier-transform

void dft_2d(float2* result, const float* source, int32_t width, int32_t height) {
	int32_t row_size = dft_size(width);

	float2* tmp = new float2[height * row_size];

	for (int32_t y = 0; y < height; ++y) {
		dft_1d(tmp + y * row_size, source + y * width, width);
	}

	float fn = static_cast<float>(height);

	for (int32_t x = 0; x < row_size; ++x) {
		for (int32_t k = 0; k < height; ++k) {
			float2 sum(0.f);

			float a = 2.f * Pi * static_cast<float>(k) / fn;

			for (int32_t t = 0; t < height; ++t) {
				int32_t g = t * row_size + x;

				float angle = a * static_cast<float>(t);

				float cos_a = std::cos(angle);
				float sin_a = std::sin(angle);

				sum.x +=  tmp[g].x * cos_a + tmp[g].y * sin_a;
				sum.y += -tmp[g].x * sin_a + tmp[g].y * cos_a;
			}

			int32_t c = k * row_size + x;
			result[c] = sum;
		}
	}

	delete[] tmp;
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

					sum.x +=  tmp[g].x * cos_a + tmp[g].y * sin_a;
					sum.y += -tmp[g].x * sin_a + tmp[g].y * cos_a;
				}

				int32_t c = k * row_size + x;
				result[c] = sum;
			}
		}
	}, 0, row_size);

	delete[] tmp;
}

}
