#pragma once

#include "image/typed_image.hpp"

namespace thread {

class Pool;

}

namespace exporting {

class Sink {
public:

	virtual ~Sink() {}

	virtual void write(const image::Image_float_4& image, thread::Pool& pool) = 0;
};

}
