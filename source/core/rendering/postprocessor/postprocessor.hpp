#pragma once

#include "image/typed_image.hpp"
#include "scene/camera/camera.hpp"

namespace thread { class Pool; }

namespace rendering { namespace postprocessor {

class Postprocessor {

public:

	Postprocessor(uint32_t num_passes = 1);
	virtual ~Postprocessor();

	virtual void init(const scene::camera::Camera& camera) = 0;

	virtual size_t num_bytes() const = 0;

	void apply(const image::Image_float_4& source,
			   image::Image_float_4& destination,
			   thread::Pool& pool);

private:

	virtual void apply(int32_t begin, int32_t end, uint32_t pass,
					   const image::Image_float_4& source,
					   image::Image_float_4& destination) = 0;

	uint32_t num_passes_;
};

}}

