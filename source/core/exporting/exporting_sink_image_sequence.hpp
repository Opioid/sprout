#ifndef SU_CORE_EXPORTING_SINK_IMAGE_SEQUENCE_HPP
#define SU_CORE_EXPORTING_SINK_IMAGE_SEQUENCE_HPP

#include <memory>
#include <string>
#include "exporting_sink.hpp"

namespace image {
class Writer;
}

namespace exporting {

class Image_sequence : public Sink {
  public:
    Image_sequence(std::string const& filename, std::unique_ptr<image::Writer> writer);

    virtual void write(const image::Float4& image, uint32_t frame,
                       thread::Pool& pool) override final;

  private:
    std::string filename_;

    std::unique_ptr<image::Writer> writer_;
};

}  // namespace exporting

#endif
