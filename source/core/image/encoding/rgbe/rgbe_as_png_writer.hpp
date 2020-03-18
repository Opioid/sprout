#ifndef SU_CORE_IMAGE_ENCODING_RGBE_AS_PNG_WRITER_HPP
#define SU_CORE_IMAGE_ENCODING_RGBE_AS_PNG_WRITER_HPP

#include "image/image_writer.hpp"

namespace image::encoding::rgbe {

class Rgbe_as_png : public image::Writer {
  public:

    virtual std::string file_extension() const final;

    virtual bool write(std::ostream& stream, Float4 const& image, thread::Pool& threads) final;

    static void transcode(Float4 const& source, Float4& destination, thread::Pool& threads);
};

class Rgbd_as_png : public image::Writer {
  public:

    virtual std::string file_extension() const final;

    virtual bool write(std::ostream& stream, Float4 const& image, thread::Pool& threads) final;

    static void transcode(Float4 const& source, Float4& destination, thread::Pool& threads);
};

}  // namespace image::encoding::rgbe

#endif
