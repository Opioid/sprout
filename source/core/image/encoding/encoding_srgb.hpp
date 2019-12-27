#ifndef SU_CORE_IMAGE_ENCODING_SRGB_HPP
#define SU_CORE_IMAGE_ENCODING_SRGB_HPP

#include "image/typed_image_fwd.hpp"

namespace image::encoding {

class Srgb {
  public:
    Srgb(bool error_diffusion) noexcept;

    ~Srgb() noexcept;

    void resize(uint32_t num_pixels) noexcept;

    byte3 const* data() const noexcept;

    void to_sRGB(Float4 const& image, int32_t begin, int32_t end) noexcept;

  protected:
    byte3* rgb_;

    uint32_t num_pixels_;

    bool error_diffusion_;
};

class Srgb_alpha {
  public:
    Srgb_alpha(bool error_diffusion, bool pre_multiplied_alpha) noexcept;

    ~Srgb_alpha() noexcept;

    void resize(uint32_t num_pixels) noexcept;

    byte4 const* data() const noexcept;

    void to_sRGB(Float4 const& image, int32_t begin, int32_t end) noexcept;

  protected:
    byte4* rgba_;

    uint32_t num_pixels_;

    bool error_diffusion_;

    bool pre_multiplied_alpha_;
};

}  // namespace image::encoding

#endif
