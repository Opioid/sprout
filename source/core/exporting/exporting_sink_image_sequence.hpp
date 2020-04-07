#ifndef SU_CORE_EXPORTING_SINK_IMAGE_SEQUENCE_HPP
#define SU_CORE_EXPORTING_SINK_IMAGE_SEQUENCE_HPP

#include "exporting_sink.hpp"

#include <string>

namespace image {
class Writer;
}

namespace exporting {

class Image_sequence final : public Sink {
  public:
    Image_sequence(std::string filename, image::Writer* writer);

    ~Image_sequence() final;

    void write(image::Float4 const& image, uint32_t frame, thread::Pool& threads) final;

  private:
    std::string filename_;

    image::Writer* writer_;
};

}  // namespace exporting

#endif
