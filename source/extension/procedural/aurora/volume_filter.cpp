#include "volume_filter.hpp"
#include "base/memory/align.hpp"
#include "base/math/vector3.inl"
#include "base/math/filter/gaussian.hpp"
#include "base/thread/thread_pool.hpp"

#include "base/math/print.hpp"
#include <iostream>

namespace procedural::aurora {

Volume_filter::Volume_filter(const int3& dimensions, float radius, float alpha,
							 uint32_t num_buckets) :
	dimensions_(dimensions),
	kernel_width_(static_cast<int32_t>(radius + 0.5f) * 2 + 1),
	kernel_(memory::allocate_aligned<K>(kernel_width_)),
	num_buckets_(num_buckets),
	scratch_(new float3*[num_buckets]) {
	const float fr = radius + 0.5f;
	math::filter::Gaussian_functor gauss(static_cast<float>(fr * fr * fr), alpha);

	const int32_t ir = static_cast<int32_t>(radius + 0.5f);

	for (int32_t x = 0, width = kernel_width_; x < width; ++x) {
		const int32_t o = -ir + x;

		const float fo = static_cast<float>(o);
		const float w = gauss(std::abs(fo * fo * fo));

		kernel_[x] = K{o, w};
	}

	for (uint32_t i = 0; i < num_buckets; ++i) {
		scratch_[i] = memory::allocate_aligned<float3>(dimensions[0] * dimensions[1]);
	}
}

Volume_filter::~Volume_filter() {
	for (uint32_t i = 0, len = num_buckets_; i < len; ++i) {
		memory::free_aligned(scratch_[i]);
	}
	delete[] scratch_;

	memory::free_aligned(kernel_);
}

void Volume_filter::filter(float3* target, thread::Pool& pool) const {
	pool.run_range([this, target]
		(uint32_t id, int32_t begin, int32_t end) {
			filter_slices(id, begin, end, target);
		}, 0, dimensions_[2]);

	pool.run_range([this, target]
		(uint32_t id, int32_t begin, int32_t end) {
			filter_z(id, begin, end, target);
		}, 0, dimensions_[0]);
}

void Volume_filter::filter_slices(uint32_t id, int32_t begin, int32_t end, float3* target) const {
	float3* scratch = scratch_[id];

	const int32_t area = dimensions_[0] * dimensions_[1];

	for (int32_t z = begin; z < end; ++z) {
		for (int32_t y = 0, height = dimensions_[1]; y < height; ++y) {
			for (int32_t x = 0, width = dimensions_[0]; x < width; ++x) {

				float3 accum(0.f);
				float  weight_sum = 0.f;

				for (int32_t ki = 0, len = kernel_width_; ki < len; ++ki) {
					const K k = kernel_[ki];
					const int32_t kx = x + k.o;

					if (kx >= 0 && kx < width) {
						const int32_t i = z * area + y * dimensions_[0] + kx;

						const float3 color = target[i];

						accum += k.w * color;
						weight_sum += k.w;
					}
				}

				const int32_t o = y * dimensions_[0] + x;

				const float3 filtered = accum / weight_sum;
				scratch[o] = filtered;
			}
		}

		for (int32_t y = 0, height = dimensions_[1]; y < height; ++y) {
			for (int32_t x = 0, width = dimensions_[0]; x < width; ++x) {

				float3 accum(0.f);
				float  weight_sum = 0.f;

				for (int32_t ki = 0, len = kernel_width_; ki < len; ++ki) {
					const K k = kernel_[ki];
					const int32_t ky = y + k.o;

					if (ky >= 0 && ky < height) {
						const int32_t i = ky * dimensions_[0] + x;

						const float3 color = scratch[i];

						accum += k.w * color;
						weight_sum += k.w;
					}
				}

				const int32_t o = z * area + y * dimensions_[0] + x;

				const float3 filtered = accum / weight_sum;
				target[o] = filtered;
			}
		}
	}
}

void Volume_filter::filter_z(uint32_t id, int32_t begin, int32_t end, float3* target) const {
	float3* scratch = scratch_[id];

	const int32_t area = dimensions_[0] * dimensions_[1];

	for (int32_t x = begin; x < end; ++x) {
		for (int32_t y = 0, height = dimensions_[1]; y < height; ++y) {
			for (int32_t z = 0, depth = dimensions_[2]; z < depth; ++z) {

				float3 accum(0.f);
				float  weight_sum = 0.f;

				for (int32_t ki = 0, len = kernel_width_; ki < len; ++ki) {
					const K k = kernel_[ki];
					const int32_t kz = z + k.o;

					if (kz >= 0 && kz < depth) {
						const int32_t i = kz * area + y * dimensions_[0] + x;

						const float3 color = target[i];

						accum += k.w * color;
						weight_sum += k.w;
					}
				}

				const int32_t o = y * dimensions_[0] + z;

				const float3 filtered = accum / weight_sum;
				scratch[o] = filtered;

			}
		}

		for (int32_t y = 0, height = dimensions_[1]; y < height; ++y) {
			for (int32_t z = 0, depth = dimensions_[2]; z < depth; ++z) {
				const int32_t i = y * dimensions_[0] + z;
				const int32_t o = z * area + y * dimensions_[0] + x;

				target[o] = scratch[i];
			}
		}
	}
}

}
