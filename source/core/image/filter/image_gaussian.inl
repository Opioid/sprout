#pragma once

#include "image_gaussian.hpp"
#include "base/math/filter/gaussian.hpp"
#include "thread/thread_pool.hpp"

namespace image { namespace filter {

template<typename T>
Gaussian<T>::Gaussian(float radius, float alpha) {
	const int32_t width = 2 * static_cast<int32_t>(radius + 0.5f) + 1;

	kernel_.resize(width);

	const float fr = radius + 0.5f;
	math::filter::Gaussian_functor gauss(fr * fr, alpha);

	const int32_t ir = static_cast<int32_t>(radius);

	for (int32_t x = 0; x < width; ++x) {
		const int32_t o = -ir + x;

		const float fo = static_cast<float>(o);
		const float w = gauss(fo * fo);

		kernel_[x] = K{o, w};
	}
}

template<typename T>
void Gaussian<T>::apply(Typed_image<T>& target, thread::Pool& pool) {
	scratch_.resize(target.description());

	const auto d = target.description().dimensions;

	// vertical

	pool.run_range([&target, d, this](uint32_t /*id*/, int32_t begin, int32_t end) {
		for (int32_t y = begin; y < end; ++y) {
			for (int32_t x = 0; x < d[0]; ++x) {
				T accum(0.f);
				float weight_sum = 0.f;
				for (auto k : kernel_) {
					int32_t kx = x + k.o;

					if (kx >= 0 && kx < d[0]) {
						T v = target.load(kx, y);
						accum += k.w * v;
						weight_sum += k.w;
					}
				}

				scratch_.store(x, y, accum / weight_sum);
			}
		}
	}, 0, d[1]);

	// horizontal

	pool.run_range([&target, d, this](uint32_t /*id*/, int32_t begin, int32_t end) {
		for (int32_t y = begin; y < end; ++y) {
			for (int32_t x = 0; x < d[0]; ++x) {
				T accum(0.f);
				float weight_sum = 0.f;
				for (auto k : kernel_) {
					int32_t ky = y + k.o;

					if (ky >= 0 && ky < d[1]) {
						T v = scratch_.load(x, ky);
						accum += k.w * v;
						weight_sum += k.w;
					}
				}

				target.store(x, y, accum / weight_sum);
			}
		}
	}, 0, d[1]);
}

}}
