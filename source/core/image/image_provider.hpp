#ifndef SU_CORE_IMAGE_PROVIDER_HPP
#define SU_CORE_IMAGE_PROVIDER_HPP

#include "encoding/png/png_reader.hpp"
#include "procedural/flakes/flakes_provider.hpp"
#include "resource/resource_provider.hpp"

namespace image {

class Image;

class Provider final : public resource::Provider<Image> {
  public:
    Provider();

    ~Provider() final;

    Image* load(std::string const& filename, Variants const& options, Resources& resources,
                std::string& resolved_name) final;

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
                Resources& resources) final;

    void increment_generation() final;

  private:
    encoding::png::Reader png_reader_;

    procedural::flakes::Provider flakes_provider_;

    std::string previous_name_;
};

}  // namespace image

#endif
