#pragma once

#include "image/typed_image.hpp"
#include <vector>

namespace image { namespace filter {

template<typename T>
class Gaussian {

public:

	Gaussian(float radius, float alpha);

	void apply(Typed_image<T>& target);

private:

	Typed_image<T> scratch_;

	struct K {
		int32_t o;
		float w;
	};

	std::vector<K> kernel_;
};

}}
