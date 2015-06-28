#pragma once

#include "exporting_sink.hpp"
#include "base/color/color.hpp"
#include "base/math/vector.hpp"
#include <string>
#include <cstdio>

namespace exporting {

class Ffmpeg : public Sink {
public:

	Ffmpeg(const std::string& filename, const math::uint2& dimensions);
	~Ffmpeg();

	virtual void write(const image::Image& image, thread::Pool& pool) final override;

private:

	void to_sRGB(const image::Image& image, uint32_t begin, uint32_t end);

//	std::string filename_;

	FILE* stream_;

	color::Color4c* rgba_;
};

}

