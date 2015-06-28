#include "exporting_sink_ffmpeg.hpp"
#include "image/image.hpp"
#include "base/color/color.inl"
#include "base/math/vector.inl"
#include "base/thread/thread_pool.hpp"
#include "base/string/string.inl"

#include <iostream>

// http://blog.mmacklin.com/2013/06/11/real-time-video-capture-with-ffmpeg/

namespace exporting {

Ffmpeg::Ffmpeg(const std::string& filename, const math::uint2& dimensions) :
	/*filename_(filename),*/ rgba_(new color::Color4c[dimensions.x * dimensions.y]) {
	// start ffmpeg telling it to expect raw rgba 720p-60hz frames
	// -i - tells it to read frames from stdin
//	const char* cmd = "ffmpeg -r 60 -f rawvideo -pix_fmt rgba -s 1280x720 -i - "
//					  "-threads 0 -preset fast -y -pix_fmt yuv420p -crf 21 -vf vflip output.mp4";

	const char* cmd = "ffmpeg -r 60 -f rawvideo -pix_fmt rgba -s 1280x720 -i - "
					  "-threads 0 -preset medium -y -pix_fmt yuv420p -crf 20 output.mp4";

	stream_ = popen(cmd, "w");
}

Ffmpeg::~Ffmpeg() {
	if (stream_) {
		pclose(stream_);
	}

	delete [] rgba_;
}

void Ffmpeg::write(const image::Image& image, thread::Pool& pool) {
	auto d = image.description().dimensions;
	pool.run_range([this, &image](uint32_t begin, uint32_t end){ to_sRGB(image, begin, end); }, 0, d.x * d.y);

	fwrite(rgba_, sizeof(color::Color4c) * d.x * d.y, 1, stream_);
}

void Ffmpeg::to_sRGB(const image::Image& image, uint32_t begin, uint32_t end) {
	for (uint32_t i = begin; i < end; ++i) {
		math::float4 color = image.at4(i);
		color.xyz = color::linear_to_sRGB(color.xyz);
		rgba_[i] = color::to_byte(color);
	}
}

}


