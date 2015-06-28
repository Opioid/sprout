#include "exporting_sink_ffmpeg.hpp"
#include "image/image.hpp"
#include "base/color/color.inl"
#include "base/thread/thread_pool.hpp"
#include <sstream>

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
	cmd << " -i - -threads 0 -preset slower -y -pix_fmt yuv420p -crf 20 ";
	cmd << filename << ".mp4";

	stream_ = popen(cmd.str().c_str(), "w");
}

Ffmpeg::~Ffmpeg() {
	if (stream_) {
		pclose(stream_);
	}
}

void Ffmpeg::write(const image::Image& image, thread::Pool& pool) {
	if (!stream_) {
		return;
	}

	auto d = image.description().dimensions;
	pool.run_range([this, &image](uint32_t begin, uint32_t end){ to_sRGB(image, begin, end); }, 0, d.x * d.y);

	fwrite(rgba_, sizeof(color::Color4c) * d.x * d.y, 1, stream_);
}

}


