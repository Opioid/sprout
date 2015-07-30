#pragma once

namespace math {

template<typename T>
class Interpolated_function {
public:

	template<typename F>
	Interpolated_function(float range_begin, float range_end, size_t num_samples, F f);

	~Interpolated_function();

	T f(float x) const;

private:

	T* samples_;
};

}
