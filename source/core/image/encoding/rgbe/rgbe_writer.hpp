#pragma once

#include "image/image_writer.hpp"

namespace image::encoding::rgbe {

class Writer : public image::Writer {
  public:
    //	Writer(math::uint2 dimensions);

    virtual std::string file_extension() const noexcept final;

    virtual bool write(std::ostream& stream, Float4 const& image,
                       thread::Pool& threads) noexcept final;
};

}  // namespace image::encoding::rgbe
