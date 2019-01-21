#ifndef SU_CORE_EXPORTING_SINK_IMAGE_SEQUENCE_HPP
#define SU_CORE_EXPORTING_SINK_IMAGE_SEQUENCE_HPP

#include <string>
#include "exporting_sink.hpp"

namespace image {
class Writer;
}

namespace exporting {

class Image_sequence : public Sink {
  public:
    Image_sequence(std::string const& filename, image::Writer* writer) noexcept;

    ~Image_sequence() noexcept override final;

    void write(image::Float4 const& image, uint32_t frame,
               thread::Pool& pool) noexcept override final;

  private:
    std::string filename_;

    image::Writer* writer_;
};

}  // namespace exporting

#endif
