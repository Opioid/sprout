#ifndef SU_CORE_IMAGE_ENCODING_EXR_WRITER_HPP
#define SU_CORE_IMAGE_ENCODING_EXR_WRITER_HPP

#include "exr.hpp"
#include "image/image_writer.hpp"

namespace image::encoding::exr {

class Writer : public image::Writer {
  public:
    Writer(bool alpha);

    std::string file_extension() const final;

    bool write(std::ostream& stream, Float4 const& image, Threads& threads) final;

  private:
    bool no_compression(std::ostream& stream, Float4 const& image) const;

    bool zip_compression(std::ostream& stream, Float4 const& image, Compression compression,
                         Threads& threads) const;

    bool half_ = true;
    bool alpha_;
};

}  // namespace image::encoding::exr

#endif
