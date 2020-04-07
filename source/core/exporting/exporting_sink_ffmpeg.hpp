#ifndef SU_CORE_EXPORTING_SINK_FFMPEG_HPP
#define SU_CORE_EXPORTING_SINK_FFMPEG_HPP

#include "exporting_sink.hpp"
#include "image/encoding/encoding_srgb.hpp"

#include <cstdio>
#include <string>

namespace exporting {

class Ffmpeg final : public Sink, image::encoding::Srgb {
  public:
    Ffmpeg(std::string const& filename, int2 dimensions, bool error_diffusion, uint32_t framerate);

    ~Ffmpeg() final;

    void write(image::Float4 const& image, uint32_t frame, thread::Pool& threads) final;

  private:
    FILE* stream_;
};

}  // namespace exporting

#endif
