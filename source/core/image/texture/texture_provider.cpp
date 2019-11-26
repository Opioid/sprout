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
#include "texture_encoding.hpp"
#include "texture_float1.hpp"
#include "texture_float3.hpp"

#include "base/debug/assert.hpp"
#include "texture_test.hpp"

namespace image::texture {

Provider::Provider(bool no_textures) noexcept : no_textures_(no_textures) {
    encoding::init();
}

Texture* Provider::load(std::string const& filename, Variant_map const& options,
                        resource::Manager& resources, std::string& resolved_name) noexcept {
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

    auto const image_res = resources.load<Image>(filename, image_options, resolved_name);
    if (!image_res.ptr) {
        logging::error("Loading texture %S: ", filename);
        return nullptr;
    }

    auto const image = image_res.ptr;

    if (Image::Type::Byte1 == image->type()) {
        return new Texture(Byte1_unorm(image->byte1()));
    } else if (Image::Type::Byte2 == image->type()) {
        if (Usage::Anisotropy == usage) {
            return new Texture(Byte2_snorm(image->byte2()));
        } else {
            return new Texture(Byte2_unorm(image->byte2()));
        }
    } else if (Image::Type::Byte3 == image->type()) {
        if (Usage::Normal == usage) {
            SOFT_ASSERT(testing::is_valid_normal_map(*image, filename));

            return new Texture(Byte3_snorm(image->byte3()));
        } else {
            return new Texture(Byte3_sRGB(image->byte3()));
        }
    } else if (Image::Type::Byte4 == image->type()) {
        return new Texture(Byte4_sRGB(image->byte4()));
    } else if (Image::Type::Short3 == image->type()) {
        return new Texture(Half3(image->short3()));
    } else if (Image::Type::Float1 == image->type()) {
        return new Texture(Float1(image->float1()));
    } else if (Image::Type::Float1_sparse == image->type()) {
        return new Texture(Float1_sparse(image->float1_sparse()));
    } else if (Image::Type::Float3 == image->type()) {
        return new Texture(Float3(image->float3()));
    }

    // We should never come here...

    return nullptr;
}

Texture* Provider::load(void const* /*data*/, std::string const& /*source_name*/,
                        Variant_map const& /*options*/, resource::Manager& /*manager*/) noexcept {
    return nullptr;
}

size_t Provider::num_bytes() const noexcept {
    return sizeof(*this);
}

size_t Provider::num_bytes(Texture const* /*resource*/) const noexcept {
    return 0;
}

}  // namespace image::texture
