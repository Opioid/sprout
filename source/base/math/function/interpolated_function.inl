#ifndef SU_BASE_MATH_INTERPOLATED_FUNCTION_INL
#define SU_BASE_MATH_INTERPOLATED_FUNCTION_INL

#include "interpolated_function.hpp"
#include "math/math.hpp"

namespace math {

template<typename T>
template<typename F>
Interpolated_function<T>::Interpolated_function(float range_begin, float range_end,
												size_t num_samples, F f) :
	range_end_(range_end), samples_(new T[num_samples + 1]) {
	float range = range_end - range_begin;

	float interval = range / static_cast<float>(num_samples - 1);

	inverse_range_ = 1.f / interval;

	float s = range_begin;
	for (size_t i = 0; i < num_samples; ++i, s += interval) {
		samples_[i] = f(s);
	}

	samples_[num_samples] = f(range_end);
}

template<typename T>
Interpolated_function<T>::~Interpolated_function() {
	delete[] samples_;
}

template<typename T>
T Interpolated_function<T>::operator()(float x) const {
	x = std::min(x, range_end_);

	float o = x * inverse_range_;
	uint32_t offset = static_cast<uint32_t>(o);

	float t = o - static_cast<float>(offset);

	return lerp(samples_[offset], samples_[offset + 1], t);
}

}

#endif
