#pragma once

#include "exporting_sink.hpp"

namespace exporting {

class Null : public Sink {
public:

	Null();
	~Null();

	virtual void write(const image::Image_float_4& image, uint32_t frame, thread::Pool& pool) final override;
};

}
