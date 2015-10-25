#pragma once

#include "exporting_sink.hpp"
#include "exporting_srgb.hpp"
#include <string>

namespace exporting {

class Image_sequence : public Sink, public Srgb {
public:

	Image_sequence(const std::string& filename, const math::uint2& dimensions);
	~Image_sequence();

	virtual void write(const image::Image_float_4& image, uint32_t frame, thread::Pool& pool) final override;

private:

	std::string filename_;
};

}
