#include "image_provider.hpp"
#include <istream>
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

namespace image {

Provider::Provider() noexcept {}

Provider::~Provider() noexcept {}

Image* Provider::load(std::string const& filename, Variant_map const& options,
                      resource::Manager& resources, std::string& resolved_name) noexcept {
    if ("proc:flakes" == filename) {
        return flakes_provider_.create_normal_map(options);
    } else if ("proc:flakes_mask" == filename) {
        return flakes_provider_.create_mask(options);
    }

    auto stream_pointer = resources.filesystem().read_stream(filename, resolved_name);
    if (!stream_pointer) {
        return nullptr;
    }

    auto& stream = *stream_pointer;

    file::Type const type = file::query_type(stream);

    if (file::Type::EXR == type) {
        return encoding::exr::Reader::read(stream);
    } else if (file::Type::PNG == type) {
        Channels channels = Channels::None;
        options.query("channels", channels);

        int32_t num_elements = 1;
        options.query("num_elements", num_elements);

        Swizzle swizzle = Swizzle::XYZW;
        options.query("swizzle", swizzle);
        bool const swap_xy = Swizzle::YXZW == swizzle;

        bool invert = false;
        options.query("invert", invert);

        return encoding::png::Reader::read(stream, channels, num_elements, swap_xy, invert);
    } else if (file::Type::RGBE == type) {
        return encoding::rgbe::Reader::read(stream);
    } else if (file::Type::SUB == type) {
        return encoding::sub::Reader::read(stream);
    } else if (file::Type::Undefined == type) {
        if ("raw" == string::suffix(filename) || "raw" == string::presuffix(filename)) {
            return encoding::raw::Reader::read(stream);
        } else if ("json" == string::suffix(filename) || "json" == string::presuffix(filename)) {
            return encoding::json::Reader::read(stream, filename);
        }
    }

    logging::push_error("Image type for %S not recognized.", filename);
    return nullptr;
}

Image* Provider::load(void const* /*data*/, std::string const& /*source_name*/,
                      Variant_map const& /*options*/, resource::Manager& /*manager*/) noexcept {
    return nullptr;
}

size_t Provider::num_bytes() const noexcept {
    return sizeof(*this);
}

size_t Provider::num_bytes(Image const* resource) const noexcept {
    return resource->num_bytes();
}

}  // namespace image
