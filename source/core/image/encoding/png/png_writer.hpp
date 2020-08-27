#ifndef SU_CORE_IMAGE_ENCODING_PNG_WRITER_HPP
#define SU_CORE_IMAGE_ENCODING_PNG_WRITER_HPP

#include "image/encoding/encoding_srgb.hpp"
#include "image/image_writer.hpp"

namespace image::encoding::png {

class Writer : public image::Writer, Srgb {
  public:
    Writer(bool error_diffusion, bool alpha, bool pre_multiplied_alpha);

    std::string file_extension() const final;

    bool write(std::ostream& stream, Float4 const& image, thread::Pool& threads) final;

    static bool write(std::string_view name, Byte3 const& image);

    static bool write(std::string_view name, Byte1 const& image);

    static bool write(std::string_view name, float const* data, int2 dimensions, float scale = 1.f,
                      bool srgb = false);

    static bool write(std::string_view name, float2 const* data, int2 dimensions,
                      float scale = 1.f);

    static bool write(std::string_view name, packed_float3 const* data, int2 dimensions,
                      float scale = 1.f);

    static bool write_heatmap(std::string_view name, float const* data, int2 dimensions,
                              thread::Pool& threads);

    static bool write_heatmap(std::string_view name, float const* data, int2 dimensions,
                              float max_value, thread::Pool& threads);
};

}  // namespace image::encoding::png

#endif
