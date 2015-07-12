#pragma once

#include "exporting_sink.hpp"
#include "exporting_srgb.hpp"
#include <string>
#include <cstdio>

namespace exporting {

class Ffmpeg : public Sink, public Srgb {
public:

	Ffmpeg(const std::string& filename, const math::uint2& dimensions, uint32_t framerate);
	~Ffmpeg();

	virtual void write(const image::Image_float_4& image, thread::Pool& pool) final override;

private:

	FILE* stream_;
};

}

