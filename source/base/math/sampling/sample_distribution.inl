#pragma once

#include "sample_distribution.hpp"

namespace math {

inline uint32_t radical_inverse_vdC_uint(uint32_t bits) {
	bits = (bits << 16) | (bits >> 16);
	bits = ((bits & 0x55555555) << 1) | ((bits & 0xAAAAAAAA) >> 1);
	bits = ((bits & 0x33333333) << 2) | ((bits & 0xCCCCCCCC) >> 2);
	bits = ((bits & 0x0F0F0F0F) << 4) | ((bits & 0xF0F0F0F0) >> 4);
	bits = ((bits & 0x00FF00FF) << 8) | ((bits & 0xFF00FF00) >> 8);

	return bits; // / 0x100000000
}

inline float radical_inverse_vdC(uint32_t bits, uint32_t r) {
	bits = (bits << 16) | (bits >> 16);
	bits = ((bits & 0x55555555) << 1) | ((bits & 0xAAAAAAAA) >> 1);
	bits = ((bits & 0x33333333) << 2) | ((bits & 0xCCCCCCCC) >> 2);
	bits = ((bits & 0x0F0F0F0F) << 4) | ((bits & 0xF0F0F0F0) >> 4);
	bits = ((bits & 0x00FF00FF) << 8) | ((bits & 0xFF00FF00) >> 8);

	bits ^= r;

	return static_cast<float>(bits) * 2.3283064365386963e-10f; // / 0x100000000
}

inline float radical_inverse_S(uint32_t i, uint32_t r) {
	for (uint32_t v = 1 << 31; i; i >>= 1, v ^= v >> 1) {
		if (i & 1) {
			r ^= v;
		}
	}

	return static_cast<float>(r) * 2.3283064365386963e-10f; // / 0x100000000
}

inline float radical_inverse_LP(uint32_t i, uint32_t r) {
	for (uint32_t v = 1 << 31; i; i >>= 1, v |= v >> 1) {
		if (i & 1) {
			r ^= v;
		}
	}

	return static_cast<float>(r) * 2.3283064365386963e-10f; // / 0x100000000
}

inline float2 hammersley(uint32_t i, uint32_t num_samples, uint32_t r) {
	return float2(static_cast<float>(i) / static_cast<float>(num_samples),
				  radical_inverse_vdC(i, r));
}

inline float2 ems(uint32_t i, uint32_t r_0, uint32_t r_1) {
	return float2(radical_inverse_vdC(i, r_0),
				  radical_inverse_S(i, r_1));
}

inline float2 thing(uint32_t i, uint32_t num_samples, uint32_t r) {
	return float2(static_cast<float>(i) / static_cast<float>(num_samples),
				  radical_inverse_LP(i, r));
}

inline void vdC(float* samples, uint32_t num_samples, uint32_t r) {
	for (uint32_t i = 0; i < num_samples; ++i) {
		samples[i] = radical_inverse_vdC(i, r);
	}
}

inline void golden_ratio(float* samples, uint32_t num_samples, float r) {
	// set the initial second coordinate
	float x = r;
	// set the second coordinates
	for (uint32_t i = 0; i < num_samples; ++i) {
		samples[i] = x;

		// increment the coordinate
		x += 0.618033988749894f;
		if (x >= 1.f) {
			--x;
		}
	}
}

inline void golden_ratio(float2* samples, uint32_t num_samples, float2 r) {
	// set the initial first coordinate
	float x = r.x;
	float min = x;
	uint32_t idx = 0;
	// set the first coordinates
	for (uint32_t i = 0; i < num_samples; ++i) {
		samples[i].y = x;
		// keep the minimum
		if (x < min) {
			min = x;
			idx = i;
		}

		// increment the coordinate
		x += 0.618033988749894f;
		if (x >= 1.f) {
			--x;
		}
	}

	// find the first Fibonacci >= N
	uint32_t f = 1;
	uint32_t fp = 1;
	uint32_t parity = 0;
	for (; f + fp < num_samples; ++parity) {
		uint32_t tmp = f;
		f += fp;
		fp = tmp;
	}

	// set the increment and decrement
	uint32_t inc = fp;
	uint32_t dec = f;
	if (parity & 1) {
		inc = f;
		dec = fp;
	}

	// permute the first coordinates
	samples[0].x = samples[idx].y;
	for (uint32_t i = 1; i < num_samples; ++i) {
		if (idx < dec) {
			idx += inc;
			if (idx >= num_samples) {
				idx -= dec;
			}
		} else {
			idx -= dec;
		}
		samples[i].x = samples[idx].y;
	}

	// set the initial second coordinate
	float y = r.y;
	// set the second coordinates
	for (uint32_t i = 0; i < num_samples; ++i) {
		samples[i].y = y;

		// increment the coordinate
		y += 0.618033988749894f;
		if (y >= 1.f) {
			--y;
		}
	}
}

}
