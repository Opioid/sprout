#include "exporting_sink_ffmpeg.hpp"
#include <sstream>
#include "base/math/vector4.inl"
#include "base/spectrum/rgb.hpp"
#include "base/thread/thread_pool.hpp"
#include "image/typed_image.hpp"

// http://blog.mmacklin.com/2013/06/11/real-time-video-capture-with-ffmpeg/

namespace exporting {

Ffmpeg::Ffmpeg(std::string const& filename, int2 dimensions, uint32_t framerate)
    : Srgb(dimensions, false) {
    // -i - tells it to read frames from stdin
    std::ostringstream cmd;

    cmd << "ffmpeg";
    cmd << " -r " << framerate;
    cmd << " -f rawvideo -pix_fmt rgb24";
    cmd << " -s " << dimensions[0] << "x" << dimensions[1];
    cmd << " -i - -threads 0 -preset veryslow -y -pix_fmt yuv420p -crf 18 ";
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

void Ffmpeg::write(image::Float4 const& image, uint32_t /*frame*/, thread::Pool& pool) {
    if (!stream_) {
        return;
    }

    auto const d = image.description().dimensions;
    pool.run_range(
        [this, &image](uint32_t /*id*/, int32_t begin, int32_t end) { to_sRGB(image, begin, end); },
        0, d[0] * d[1]);

    fwrite(rgb_, sizeof(byte3) * static_cast<size_t>(d[0] * d[1]), 1, stream_);
}

}  // namespace exporting
