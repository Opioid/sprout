#include "exporting_sink_ffmpeg.hpp"
#include "image/image.hpp"
#include "base/color/color.inl"
#include "base/thread/thread_pool.hpp"
#include <sstream>

#include <iostream>

// http://blog.mmacklin.com/2013/06/11/real-time-video-capture-with-ffmpeg/

namespace exporting {

Ffmpeg::Ffmpeg(const std::string& filename, const math::uint2& dimensions, uint32_t framerate) : Srgb(dimensions) {
	// start ffmpeg telling it to expect raw rgba 720p-60hz frames
	// -i - tells it to read frames from stdin
	std::ostringstream cmd;

	cmd << "ffmpeg";
	cmd << " -r " << framerate;
	cmd << " -f rawvideo -pix_fmt rgba";
	cmd << " -s " << dimensions.x << "x" << dimensions.y;
	cmd << " -i - -threads 0 -preset veryslow -y -pix_fmt yuv420p -crf 19 ";
	cmd << filename << ".mp4";

#ifdef WIN32
	stream_ = _popen(cmd.str().c_str(), "wb");
#else
	stream_ = popen(cmd.str().c_str(), "w");
#endif
}

Ffmpeg::~Ffmpeg() {
	if (stream_) {
#ifdef WIN32
		_pclose(stream_);
#else
		pclose(stream_);
#endif
	}
}

void Ffmpeg::write(const image::Image_float_4& image, thread::Pool& pool) {
	if (!stream_) {
		return;
	}

	std::cout << "Ffmpeg before pool" << std::endl;

	auto d = image.description().dimensions;
	pool.run_range([this, &image](uint32_t begin, uint32_t end){ to_sRGB(image, begin, end); }, 0, d.x * d.y);

	std::cout << "Ffmpeg after pool" << std::endl;

	fwrite(rgba_, sizeof(color::Color4c) * d.x * d.y, 1, stream_);

	std::cout << "Ffmpeg after write" << std::endl;
}

}


