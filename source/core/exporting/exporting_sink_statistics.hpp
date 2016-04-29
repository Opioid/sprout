#pragma once

#include "exporting_sink.hpp"

namespace image { class Writer; }

namespace exporting {

class Statistics : public Sink {
public:

	Statistics();
	~Statistics();

	virtual void write(const image::Image_float_4& image, uint32_t frame,
					   thread::Pool& pool) final override;
};

}
