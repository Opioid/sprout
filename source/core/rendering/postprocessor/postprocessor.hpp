#pragma once

#include "scene/camera/camera.hpp"

namespace image { class Image_float_4; }

namespace rendering { namespace postprocessor {

class Postprocessor {

public:

	~Postprocessor();

	virtual void init(const scene::camera::Camera& camera) = 0;

	virtual void apply(int32_t begin, int32_t end,
					   const image::Image_float_4& source,
					   image::Image_float_4& destination) const = 0;

};

}}

