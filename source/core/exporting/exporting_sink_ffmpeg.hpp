#pragma once

#include "exporting_sink.hpp"
#include "image/encoding/encoding_srgb.hpp"
#include <string>
#include <cstdio>

namespace exporting {

class Ffmpeg : public Sink, public image::encoding::Srgb {
public:

	Ffmpeg(const std::string& filename, const math::int2& dimensions, uint32_t framerate);
	~Ffmpeg();

	virtual void write(const image::Image_float_4& image, uint32_t frame, thread::Pool& pool) final override;

private:

	FILE* stream_;
};

}
