#pragma once

#include "image/typed_image.hpp"

namespace image { namespace filter {

template<typename T>
class Gaussian {

public:

	Gaussian(float radius, float alpha);

	void apply(const Typed_image<T>& source,
			   Typed_image<T>& destination);

private:

	Typed_image<T> scratch_;

	struct K {
		int32_t o;
		float w;
	};

	std::vector<K> kernel_;

};

}}
