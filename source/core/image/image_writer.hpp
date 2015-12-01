#pragma once

#include "typed_image.hpp"
#include <ostream>

namespace thread { class Pool; }

namespace image {

class Writer {
public:

	virtual bool write(std::ostream& stream, const image::Image_float_4& image, thread::Pool& pool) = 0;

};

}
