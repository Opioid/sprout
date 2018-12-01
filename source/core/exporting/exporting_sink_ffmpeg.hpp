#pragma once

#include <cstdio>
#include <string>
#include "exporting_sink.hpp"
#include "image/encoding/encoding_srgb.hpp"

namespace exporting {

class Ffmpeg : public Sink, image::encoding::Srgb {
  public:
    Ffmpeg(std::string const& filename, int2 dimensions, bool error_diffusion, uint32_t framerate);

    ~Ffmpeg();

    void write(image::Float4 const& image, uint32_t frame, thread::Pool& pool) override final;

  private:
    FILE* stream_;
};

}  // namespace exporting
