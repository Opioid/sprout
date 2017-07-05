#pragma once

#include "exporting_sink.hpp"
#include <string>

namespace image { class Writer; }

namespace exporting {

class Image_sequence : public Sink {

public:

	Image_sequence(const std::string& filename, image::Writer* writer);
	~Image_sequence();

	virtual void write(const image::Float4& image, uint32_t frame,
					   thread::Pool& pool) override final;

private:

	std::string filename_;

	image::Writer* writer_;
};

}
