#ifndef SU_CORE_IMAGE_ENCODING_SRGB_HPP
#define SU_CORE_IMAGE_ENCODING_SRGB_HPP

#include "image/typed_image_fwd.hpp"

namespace image::encoding {

class Srgb {
  public:
    Srgb(int2 dimensions, bool error_diffusion);

    ~Srgb();

    byte3 const* data() const;

    void to_sRGB(Float4 const& image, int32_t begin, int32_t end);

  protected:
    byte3* rgb_;

    bool error_diffusion_;
};

class Srgb_alpha {
  public:
    Srgb_alpha(int2 dimensions, bool error_diffusion, bool pre_multiplied_alpha);

    ~Srgb_alpha();

    byte4 const* data() const;

    void to_sRGB(Float4 const& image, int32_t begin, int32_t end);

  protected:
    byte4* rgba_;

    bool error_diffusion_;

    bool pre_multiplied_alpha_;
};

}  // namespace image::encoding

#endif
