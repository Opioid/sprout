#pragma once

#include "exporting_sink.hpp"
#include "base/color/color.hpp"
#include "base/math/vector.hpp"
#include <string>

namespace exporting {

class Image_sequence : public Sink {
public:

	Image_sequence(const std::string& filename, const math::uint2& dimensions);
	~Image_sequence();

	virtual void write(const image::Image& image, thread::Pool& pool) final override;

private:

	void to_sRGB(const image::Image& image, uint32_t begin, uint32_t end);

	std::string filename_;

	uint32_t current_frame_;

	color::Color4c* rgba_;
};

}
