#pragma once

#include "image_gaussian.hpp"
#include "base/math/filter/gaussian.hpp"

namespace image { namespace filter {

template<typename T>
Gaussian<T>::Gaussian(float radius, float alpha) {
	int32_t width = 2 * static_cast<int32_t>(radius + 0.5f) + 1;

	kernel_.resize(width);

	float fr = radius + 0.5f;
	math::filter::Gaussian_functor gauss(static_cast<float>(fr * fr), alpha);

	int32_t ir = static_cast<int32_t>(radius);

	for (int32_t x = 0; x < width; ++x) {
		int32_t o = -ir + x;

		float fo = static_cast<float>(o);
		float w = gauss(fo * fo);

		kernel_[x] = K{o, w};
	}
}

template<typename T>
void Gaussian<T>::apply(const Typed_image<T>& source,
						Typed_image<T>& destination) {

	scratch_.resize(destination.description());

	auto d = destination.description().dimensions;

	// vertical

	int32_t begin = 0;
	int32_t end   = destination.area();

	for (int32_t i = begin; i < end; ++i) {
		int2 c = source.coordinates_2(i);

		T accum(0.f);
		float weight_sum = 0.f;
		for (auto& k : kernel_) {
			int32_t kx = c.x + k.o;

			if (kx >= 0 && kx < d.x) {
				T v = source.load(kx, c.y);
				accum += k.w * v;
				weight_sum += k.w;
			}
		}

		scratch_.store(i, accum / weight_sum);
	}

	// horizontal

	for (int32_t i = begin; i < end; ++i) {
		int2 c = source.coordinates_2(i);

		T accum(0.f);
		float weight_sum = 0.f;
		for (auto& k : kernel_) {
			int32_t ky = c.y + k.o;

			if (ky >= 0 && ky < d.y) {
				T v = scratch_.load(c.x, ky);
				accum += k.w * v;
				weight_sum += k.w;
			}
		}

		destination.store(i, accum / weight_sum);
	}
}

}}
