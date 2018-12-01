#ifndef SU_CORE_IMAGE_ENCODING_SRGB_HPP
#define SU_CORE_IMAGE_ENCODING_SRGB_HPP

#include "image/typed_image_fwd.hpp"

namespace image::encoding {

class Srgb {
  public:
    Srgb(int2 dimensions, bool dither);

    ~Srgb();

    byte3 const* data() const;

    void to_sRGB(Float3 const& image, int32_t begin, int32_t end);
    void to_sRGB(Float4 const& image, int32_t begin, int32_t end);

    void to_byte(Float3 const& image, int32_t begin, int32_t end);
    void to_byte(Float4 const& image, int32_t begin, int32_t end);

  protected:
    byte3* rgb_;

    bool dither_;
};

class Srgb_alpha {
  public:
    Srgb_alpha(int2 dimensions);

    ~Srgb_alpha();

    byte4 const* data() const;

    void to_sRGB(Float3 const& image, int32_t begin, int32_t end);
    void to_sRGB(Float4 const& image, int32_t begin, int32_t end);

    void to_byte(Float3 const& image, int32_t begin, int32_t end);
    void to_byte(Float4 const& image, int32_t begin, int32_t end);

  protected:
    byte4* rgba_;
};

}  // namespace image::encoding

#endif
