#include "interpolated_function.hpp"

namespace math {

template<typename T>
template<typename F>
Interpolated_function<T>::Interpolated_function(float range_begin, float range_end, size_t num_samples, F f) :
	samples_(new T[num_samples + 1]) {
	float range = range_end - range_begin;

	float interval = range / static_cast<float>(num_samples);

	float s = range_begin;
	for (size_t i = 0; i < num_samples; ++i, s += interval) {
		samples_[i] = f(s);
	}

	size_t t = 0;
}

template<typename T>
Interpolated_function<T>::~Interpolated_function() {
	delete [] samples_;
}

template<typename T>
T Interpolated_function<T>::f(float x) const {
	return T(0);
}

}
