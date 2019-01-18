#include "texture_provider.hpp"
#include "base/math/vector4.inl"
#include "base/memory/variant_map.inl"
#include "image/channels.hpp"
#include "image/image.hpp"
#include "image/image_provider.hpp"
#include "logging/logging.hpp"
#include "resource/resource_manager.inl"
#include "resource/resource_provider.inl"
#include "texture_byte_1_unorm.hpp"
#include "texture_byte_2_snorm.hpp"
#include "texture_byte_2_unorm.hpp"
#include "texture_byte_3_snorm.hpp"
#include "texture_byte_3_srgb.hpp"
#include "texture_byte_3_unorm.hpp"
#include "texture_encoding.hpp"
#include "texture_float_1.hpp"
#include "texture_float_3.hpp"

#include "base/debug/assert.hpp"
#include "texture_test.hpp"

namespace image::texture {

Provider::Provider() noexcept : resource::Provider<Texture>("Texture") {
    encoding::init();
}

Texture* Provider::load(std::string const& filename, Variant_map const& options,
                        resource::Manager& manager) {
    Channels channels = Channels::XYZ;

    Usage usage = Usage::Undefined;
    options.query("usage", usage);

    bool invert = false;

    if (Usage::Mask == usage) {
        channels = Channels::W;
    } else if (Usage::Anisotropy == usage) {
        channels = Channels::XY;
    } else if (Usage::Surface == usage) {
        channels = Channels::XY;
    } else if (Usage::Roughness == usage) {
        channels = Channels::X;
    } else if (Usage::Specularity == usage) {
        channels = Channels::X;
        invert   = true;
    }

    memory::Variant_map image_options;
    image_options.set("channels", channels);
    image_options.inherit_except(options, "usage");

    if (invert) {
        image_options.set("invert", invert);
    }

    try {
        auto image = manager.load<Image>(filename, image_options);
        if (!image) {
            logging::error("Loading texture \"" + filename + "\": Undefined error.");
            return nullptr;
        }

        if (Image::Type::Byte1 == image->description().type) {
            return new Byte1_unorm(*image);
        } else if (Image::Type::Byte2 == image->description().type) {
            if (Usage::Anisotropy == usage) {
                return new Byte2_snorm(*image);
            } else {
                return new Byte2_unorm(*image);
            }
        } else if (Image::Type::Byte3 == image->description().type) {
            if (Usage::Normal == usage) {
                SOFT_ASSERT(testing::is_valid_normal_map(*image, filename));

                return new Byte3_snorm(*image);
            } else if (Usage::Surface == usage) {
                return new Byte3_unorm(*image);
            } else {
                return new Byte3_sRGB(*image);
            }
        } else if (Image::Type::Float1 == image->description().type) {
            return new Float1(*image);
        } else if (Image::Type::Float1_sparse == image->description().type) {
            return new Float1_sparse(*image);
        } else if (Image::Type::Float3 == image->description().type) {
            return new Float3(*image);
        } else {
            logging::error("Loading texture \"" + filename + "\": Image is of unknown type.");
        }
    } catch (const std::exception& e) {
        logging::error("Loading texture \"" + filename + "\": " + e.what() + ".");
    }

    return nullptr;
}

Texture* Provider::load(void const* /*data*/, std::string_view /*mount_folder*/,
                        Variant_map const& /*options*/, resource::Manager& /*manager*/) {
    return nullptr;
}

size_t Provider::num_bytes() const noexcept {
    return sizeof(*this);
}

}  // namespace image::texture
