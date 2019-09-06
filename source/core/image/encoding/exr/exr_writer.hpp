#ifndef SU_CORE_IMAGE_ENCODING_EXR_WRITER_HPP
#define SU_CORE_IMAGE_ENCODING_EXR_WRITER_HPP

#include "exr.hpp"
#include "image/image_writer.hpp"

namespace image::encoding::exr {

class Writer : public image::Writer {
  public:
    Writer(bool alpha);

    std::string file_extension() const override final;

    bool write(std::ostream& stream, Float4 const& image, thread::Pool& pool) override final;

  private:
    bool no_compression(std::ostream& stream, Float4 const& image) const noexcept;

    bool zip_compression(std::ostream& stream, Float4 const& image, Compression compression) const
        noexcept;

    bool alpha_;
};

}  // namespace image::encoding::exr

#endif
