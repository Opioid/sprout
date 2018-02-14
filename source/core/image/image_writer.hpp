#pragma once

#include "typed_image_fwd.hpp"
#include <ostream>

namespace thread { class Pool; }

namespace image {

class Writer {

public:

	virtual ~Writer();

	virtual std::string file_extension() const = 0;

	virtual bool write(std::ostream& stream, const Float4& image, thread::Pool& pool) = 0;
};

}
