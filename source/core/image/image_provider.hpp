#ifndef SU_CORE_IMAGE_PROVIDER_HPP
#define SU_CORE_IMAGE_PROVIDER_HPP

#include "procedural/flakes/flakes_provider.hpp"
#include "resource/resource_provider.hpp"

namespace image {

class Image;

class Provider final : public resource::Provider<Image> {
  public:
    Provider() noexcept;

    ~Provider() noexcept override final;

    Image* load(std::string const& filename, Variants const& options, Resources& resources,
                std::string& resolved_name) noexcept override final;

    struct Description {
        enum class Pixel_type { Byte = 0, Short, Float };

        Pixel_type pixel_type;

        uint32_t num_channels;

        int3 dimensions;

        int32_t num_elements;

        uint32_t stride;

        char const* data;
    };

    Image* load(void const* data, std::string const& source_name, Variants const& options,
                Resources& resources) noexcept override final;

    size_t num_bytes() const noexcept override final;

    size_t num_bytes(Image const* resource) const noexcept override final;

  private:
    procedural::flakes::Provider flakes_provider_;
};

}  // namespace image

#endif
