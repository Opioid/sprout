#include "dft.hpp"
#include "math/math.hpp"

namespace math {

size_t dft_size(size_t num) {
	return (num / 2 + 1) * 2;
}

void dft_1d(float* result, const float* source, size_t num) {
	float fn = static_cast<float>(num);

	for (size_t k = 0, len = num / 2; k <= len; ++k) {
		size_t r = k * 2;
		size_t i = r + 1;

		result[r] = 0.f;
		result[i] = 0.f;

		float a = -2.f * Pi / fn * static_cast<float>(k);

		for (size_t x = 0; x < num; ++x) {
			float b = a * static_cast<float>(x);
			result[r] += source[x] * std::cos(b);
			result[i] += source[x] * std::sin(b);
		}

		// normalization
//		result[r] *= (k == 0 || k == len) ? 1.f / fn : 2.f / fn;
//		result[i] *= 2.f / fn;
	}
}

void idft_1d(float* result, const float* source, size_t num) {
	float fn = static_cast<float>(num);

	for (size_t x = 0; x < num; ++x) {
		result[x] = source[0];

		float a = -2.f * Pi / fn * x;

		for (size_t k = 1, len = num / 2; k <= len; ++k) {
			size_t r = k * 2;
			size_t i = r + 1;

			float b = a * static_cast<float>(k);

			result[x] += source[r] * std::cos(b) + source[i] * std::sin(b);
		}
	}
}

void dft_2d(float* result, const float* source, size_t width, size_t height) {
	size_t row_size = dft_size(width);

	float* tmp = new float[height * row_size];

	for (size_t y = 0; y < height; ++y) {
		dft_1d(tmp + y * row_size, source + y * width, width);
	}

	// https://www.nayuki.io/page/how-to-implement-the-discrete-fourier-transform


	delete [] tmp;
}

}
