#ifndef SU_CORE_IMAGE_ENCODING_SRGB_HPP
#define SU_CORE_IMAGE_ENCODING_SRGB_HPP

#include "image/typed_image_fwd.hpp"

namespace image::encoding {

class Srgb {
  public:
    Srgb(bool error_diffusion, bool alpha, bool pre_multiplied_alpha);

    ~Srgb();

    bool alpha() const;

    void resize(uint32_t num_pixels);

    char* data();

    void to_sRGB(Float4 const& image, int32_t begin, int32_t end);

  protected:
    char* buffer_;

    uint32_t num_bytes_;

    bool error_diffusion_;

    bool alpha_;

    bool pre_multiplied_alpha_;
};

}  // namespace image::encoding

#endif
