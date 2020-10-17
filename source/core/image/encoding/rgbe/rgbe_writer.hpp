#ifndef SU_CORE_IMAGE_ENCODING_RGBE_WRITER_HPP
#define SU_CORE_IMAGE_ENCODING_RGBE_WRITER_HPP

#include "image/image_writer.hpp"

namespace image::encoding::rgbe {

class Writer : public image::Writer {
  public:
    //	Writer(math::uint2 dimensions);

    std::string file_extension() const final;

    bool write(std::ostream& stream, Float4 const& image, Threads& threads) final;
};

}  // namespace image::encoding::rgbe

#endif
