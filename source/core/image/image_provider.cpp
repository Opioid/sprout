#include "image_provider.hpp"
#include "base/math/vector4.inl"
#include "base/memory/variant_map.inl"
#include "encoding/exr/exr_reader.hpp"
#include "encoding/json/json_reader.hpp"
#include "encoding/png/png_reader.hpp"
#include "encoding/raw/raw_reader.hpp"
#include "encoding/rgbe/rgbe_reader.hpp"
#include "encoding/sub/sub_image_reader.hpp"
#include "file/file.hpp"
#include "file/file_system.hpp"
#include "image.hpp"
#include "logging/logging.hpp"
#include "resource/resource_manager.hpp"
#include "resource/resource_provider.inl"
#include "string/string.hpp"

#include <istream>

namespace image {

Provider::Provider() = default;

Provider::~Provider() = default;

Image* Provider::load(std::string const& filename, Variants const& options, Resources& resources,
                      std::string& resolved_name) {
    if ("proc:flakes" == filename) {
        return flakes_provider_.create_normal_map(options);
    }

    if ("proc:flakes_mask" == filename) {
        return flakes_provider_.create_mask(options);
    }

    auto stream = resources.filesystem().read_stream(filename, resolved_name);
    if (!stream) {
        return nullptr;
    }

    std::string const previous_name = previous_name_;

    previous_name_ = resolved_name;

    file::Type const type = file::query_type(*stream);

    if (file::Type::EXR == type) {
        return encoding::exr::Reader::read(*stream);
    }

    if (file::Type::PNG == type) {
        Channels channels = Channels::None;
        options.query("channels", channels);

        int32_t num_elements = 1;
        options.query("num_elements", num_elements);

        Swizzle swizzle = Swizzle::XYZW;
        options.query("swizzle", swizzle);
        bool const swap_xy = Swizzle::YXZW == swizzle;

        bool invert = false;
        options.query("invert", invert);

        if (previous_name == resolved_name) {
            return png_reader_.create_from_buffer(channels, num_elements, swap_xy, invert);
        }

        return png_reader_.read(*stream, channels, num_elements, swap_xy, invert);
    }

    if (file::Type::RGBE == type) {
        return encoding::rgbe::Reader::read(*stream);
    }

    if (file::Type::SUB == type) {
        return encoding::sub::Reader::read(*stream);
    }

    if (file::Type::Undefined == type) {
        if ("raw" == string::suffix(filename) || "raw" == string::presuffix(filename)) {
            return encoding::raw::Reader::read(*stream);
        }

        if ("json" == string::suffix(filename) || "json" == string::presuffix(filename)) {
            return encoding::json::Reader::read(*stream, filename);
        }
    }

    logging::push_error("Image type for %S not recognized.", filename);
    return nullptr;
}

Image* Provider::load(void const* data, std::string const& /*source_name*/,
                      Variants const& /*options*/, resource::Manager& /*manager*/) {
    Description const& desc = *reinterpret_cast<Description const*>(data);

    Image* image = nullptr;

    uint32_t pixel_width = 0;

    if (Description::Pixel_type::Byte == desc.pixel_type) {
        switch (desc.num_channels) {
            case 1:
                image = new Image(Byte1(image::Description(desc.dimensions, desc.num_elements)));
                break;
            case 2:
                image = new Image(Byte2(image::Description(desc.dimensions, desc.num_elements)));
                break;
            case 3:
                image = new Image(Byte3(image::Description(desc.dimensions, desc.num_elements)));
                break;
            case 4:
                image = new Image(Byte4(image::Description(desc.dimensions, desc.num_elements)));
                break;
            default:
                break;
        }

        pixel_width = desc.num_channels * uint32_t(sizeof(uint8_t));
    } else if (Description::Pixel_type::Float == desc.pixel_type) {
        switch (desc.num_channels) {
            case 1:
                image = new Image(Float1(image::Description(desc.dimensions, desc.num_elements)));
                break;
            case 2:
                image = new Image(Float2(image::Description(desc.dimensions, desc.num_elements)));
                break;
            case 3:
                image = new Image(Float3(image::Description(desc.dimensions, desc.num_elements)));
                break;
            case 4:
                image = new Image(Float4(image::Description(desc.dimensions, desc.num_elements)));
                break;
            default:
                break;
        }

        pixel_width = desc.num_channels * uint32_t(sizeof(float));
    }

    if (!image) {
        return nullptr;
    }

    char* image_data = image->data();

    char const* source_data = desc.data;

    uint64_t const num_pixels = image->description().num_pixels();

    uint32_t const stride = desc.stride;

    if (stride == pixel_width) {
        std::copy(source_data, source_data + num_pixels * uint64_t(pixel_width), image_data);

        // Flip Y
        //        int2 const d = image->description().dimensions_2();

        //        int32_t const row = d[0] * int32_t(pixel_width);

        //        for (int32_t y = d[1] - 1, iy = 0; y >= 0; --y, ++iy) {
        //            char const* source_row = source_data + y * row;
        //            std::copy(source_row, source_row + row, image_data + iy * row);
        //        }
    } else {
        for (uint64_t i = 0; i < num_pixels;
             ++i, source_data += stride, image_data += pixel_width) {
            std::copy(source_data, source_data + pixel_width, image_data);
        }
    }

    return image;
}

}  // namespace image
