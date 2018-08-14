#include "image_provider.hpp"
#include <istream>
#include "base/math/vector4.inl"
#include "base/memory/variant_map.inl"
#include "encoding/json/json_reader.hpp"
#include "encoding/raw/raw_reader.hpp"
#include "encoding/rgbe/rgbe_reader.hpp"
#include "file/file.hpp"
#include "file/file_system.hpp"
#include "resource/resource_manager.hpp"
#include "resource/resource_provider.inl"
#include "string/string.hpp"

namespace image {

Provider::Provider() noexcept : resource::Provider<Image>("Image") {}

Provider::~Provider() noexcept {}

std::shared_ptr<Image> Provider::load(std::string const&         filename,
                                      memory::Variant_map const& options,
                                      resource::Manager&         manager) {
    if ("proc:flakes" == filename) {
        return flakes_provider_.create_normal_map(options);
    } else if ("proc:flakes_mask" == filename) {
        return flakes_provider_.create_mask(options);
    }

    auto stream_pointer = manager.filesystem().read_stream(filename);

    auto& stream = *stream_pointer;

    file::Type const type = file::query_type(stream);

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

        return png_reader_.read(stream, channels, num_elements, swap_xy, invert);
    } else if (file::Type::RGBE == type) {
        encoding::rgbe::Reader reader;
        return reader.read(stream);
    } else if (file::Type::Undefined == type) {
        if ("raw" == string::suffix(filename)) {
            encoding::raw::Reader reader;
            return reader.read(stream);
        }

        encoding::json::Reader reader;
        return reader.read(stream);
    }

    throw std::runtime_error("Image type for \"" + filename + "\" not recognized");
}

std::shared_ptr<Image> Provider::load(void const* /*data*/, std::string_view /*mount_folder*/,
                                      memory::Variant_map const& /*options*/,
                                      resource::Manager& /*manager*/) {
    return nullptr;
}

size_t Provider::num_bytes() const noexcept {
    return sizeof(*this);
}

}  // namespace image
