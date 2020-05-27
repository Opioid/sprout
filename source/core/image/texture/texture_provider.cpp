#include "texture_provider.hpp"
#include "base/math/vector4.inl"
#include "base/memory/variant_map.inl"
#include "image/channels.hpp"
#include "image/image.hpp"
#include "image/image_provider.hpp"
#include "logging/logging.hpp"
#include "resource/resource_manager.inl"
#include "resource/resource_provider.inl"
#include "texture.inl"
#include "texture_byte1_unorm.hpp"
#include "texture_byte2_snorm.hpp"
#include "texture_byte2_unorm.hpp"
#include "texture_byte3_snorm.hpp"
#include "texture_byte3_srgb.hpp"
#include "texture_byte3_unorm.hpp"
#include "texture_byte4_srgb.hpp"
#include "texture_float1.hpp"
#include "texture_float2.hpp"
#include "texture_float3.hpp"

#include "base/debug/assert.hpp"
#include "texture_test.hpp"

namespace image::texture {

Provider::Provider(bool no_textures) : no_textures_(no_textures) {}

Texture* Provider::load(std::string const& filename, Variants const& options, Resources& resources,
                        std::string& resolved_name) {
    if (no_textures_) {
        return nullptr;
    }

    Channels channels = Channels::XYZ;

    Usage usage = Usage::Undefined;
    options.query("usage", usage);

    bool invert = false;

    if (Usage::Color_with_alpha == usage) {
        channels = Channels::XYZW;
    } else if (Usage::Mask == usage) {
        channels = Channels::W;
    } else if (Usage::Anisotropy == usage) {
        channels = Channels::XY;
    } else if (Usage::Surface == usage) {
        channels = Channels::XY;
    } else if (Usage::Roughness == usage) {
        channels = Channels::X;
    } else if (Usage::Gloss == usage) {
        channels = Channels::X;
        invert   = true;
    } else if (Usage::Gloss_in_alpha == usage) {
        channels = Channels::W;
        invert   = true;
    } else if (Usage::Normal == usage) {
        channels = Channels::XYZ;
    }

    memory::Variant_map image_options;
    image_options.set("channels", channels);
    image_options.inherit_except(options, "usage");

    if (invert) {
        image_options.set("invert", invert);
    }

    uint32_t const image_id = decode_name(filename);

    auto const image_res = resource::Null != image_id
                               ? resources.get<Image>(image_id)
                               : resources.load<Image>(filename, image_options, resolved_name);

    if (!image_res.ptr) {
        logging::error("Loading texture %S: ", filename);
        return nullptr;
    }

    auto const image = image_res.ptr;

    if (Image::Type::Byte1 == image->type()) {
        return new Texture(Byte1_unorm(image->byte1()));
    }

    if (Image::Type::Byte2 == image->type()) {
        if (Usage::Anisotropy == usage) {
            return new Texture(Byte2_snorm(image->byte2()));
        }

        return new Texture(Byte2_unorm(image->byte2()));
    }

    if (Image::Type::Byte3 == image->type()) {
        if (Usage::Normal == usage) {
            SOFT_ASSERT(testing::is_valid_normal_map(*image, filename));

            return new Texture(Byte3_snorm(image->byte3()));
        }

        return new Texture(Byte3_sRGB(image->byte3()));
    }

    if (Image::Type::Byte4 == image->type()) {
        return new Texture(Byte4_sRGB(image->byte4()));
    }

    if (Image::Type::Short3 == image->type()) {
        return new Texture(Half3(image->short3()));
    }

    if (Image::Type::Float1 == image->type()) {
        return new Texture(Float1(image->float1()));
    }

    if (Image::Type::Float1_sparse == image->type()) {
        return new Texture(Float1_sparse(image->float1_sparse()));
    }

    if (Image::Type::Float2 == image->type()) {
        return new Texture(Float2(image->float2()));
    }

    if (Image::Type::Float3 == image->type()) {
        return new Texture(Float3(image->float3()));
    }

    // We should never come here...

    return nullptr;
}

Texture* Provider::load(void const* /*data*/, std::string const& /*source_name*/,
                        Variants const& /*options*/, resource::Manager& /*resources*/) {
    return nullptr;
}

std::string Provider::encode_name(uint32_t image_id) {
    return "tex:" + std::to_string(image_id);
}

uint32_t Provider::decode_name(std::string const& name) {
    if ("tex:" == name.substr(0, 4)) {
        return uint32_t(std::atoi(name.substr(4).c_str()));
    }

    return resource::Null;
}

}  // namespace image::texture
