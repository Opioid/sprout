#pragma once

#include "base/color/color.hpp"
#include "base/math/vector.hpp"

namespace image {

class Image;

}

namespace exporting {

class Srgb {
public:

	Srgb(const math::uint2& dimensions);
	virtual ~Srgb();

protected:

	void to_sRGB(const image::Image& image, uint32_t begin, uint32_t end);

	color::Color4c* rgba_;
};

}

