#ifndef SU_BASE_MATH_INTERPOLATED_FUNCTION_HPP
#define SU_BASE_MATH_INTERPOLATED_FUNCTION_HPP

#include <cstddef>

namespace math {

template<typename T>
class Interpolated_function {

public:

	template<typename F>
	Interpolated_function(float range_begin, float range_end, size_t num_samples, F f);

	~Interpolated_function();

	T operator()(float x) const;

private:

	float range_end_;

	float inverse_range_;

	T* samples_;
};

}

#endif
