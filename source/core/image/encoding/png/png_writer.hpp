#ifndef SU_CORE_IMAGE_ENCODING_PNG_WRITER_HPP
#define SU_CORE_IMAGE_ENCODING_PNG_WRITER_HPP

#include "image/encoding/encoding_srgb.hpp"
#include "image/image_writer.hpp"

namespace image::encoding::png {

class Writer : public image::Writer, Srgb {
  public:
    Writer(int2 dimensions, bool error_diffusion);

    std::string file_extension() const override final;

    bool write(std::ostream& stream, Float4 const& image, thread::Pool& threads) override final;

    bool write_heatmap(std::string_view name, float const* data, int2 dimensions, float max_value,
                       thread::Pool& threads) noexcept;

    static bool write(std::string_view name, Byte3 const& image);

    static bool write(std::string_view name, Byte1 const& image);

    static bool write(std::string_view name, float const* data, int2 dimensions, float scale = 1.f,
                      bool gamma = false);

    static bool write(std::string_view name, float2 const* data, int2 dimensions,
                      float scale = 1.f);

    static bool write(std::string_view name, packed_float3 const* data, int2 dimensions,
                      float scale = 1.f);

    static bool write_heatmap(std::string_view name, uint32_t const* data, int2 dimensions);

    static bool write_heatmap(std::string_view name, float const* data, int2 dimensions);

    static bool write_heatmap(std::string_view name, float const* data, int2 dimensions,
                              float max_value);
};

class Writer_alpha : public image::Writer, Srgb_alpha {
  public:
    Writer_alpha(int2 dimensions, bool error_diffusion, bool pre_multiplied_alpha);

    std::string file_extension() const override final;

    bool write(std::ostream& stream, Float4 const& image, thread::Pool& threads) override final;
};

}  // namespace image::encoding::png

#endif
