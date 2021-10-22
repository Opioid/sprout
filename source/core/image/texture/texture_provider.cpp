#include "texture_provider.hpp"
#include "base/math/vector4.inl"
#include "base/memory/variant_map.inl"
#include "image/channels.hpp"
#include "image/image.hpp"
#include "image/image_provider.hpp"
#include "logging/logging.hpp"
#include "resource/resource_manager.inl"
#include "texture.inl"

#include "base/debug/assert.hpp"

#include <charconv>

namespace image::texture {

Texture Provider::load(std::string const& filename, Variants const& options, float2 scale,
                       Resources& resources) {
    Swizzle swizzle = options.query("swizzle", Swizzle::Undefined);

    Usage const usage = options.query("usage", Usage::Undefined);

    bool color = false;

    if (Usage::Color == usage || Usage::Emission == usage) {
        if (Swizzle::Undefined == swizzle) {
            swizzle = Swizzle::XYZ;
        }
        color = true;
    } else if (Usage::Color_with_alpha == usage) {
        if (Swizzle::Undefined == swizzle) {
            swizzle = Swizzle::XYZW;
        }
        color = true;
    } else if (Usage::Mask == usage) {
        if (Swizzle::Undefined == swizzle) {
            swizzle = Swizzle::W;
        }
    } else if (Usage::Surface == usage) {
        if (Swizzle::Undefined == swizzle) {
            swizzle = Swizzle::XY;
        }
    } else if (Usage::Roughness == usage) {
        if (Swizzle::Undefined == swizzle) {
            swizzle = Swizzle::X;
        }
    } else if (Usage::Normal == usage) {
        if (Swizzle::Undefined == swizzle) {
            swizzle = Swizzle::XY;
        }
    }

    if (Swizzle::Undefined == swizzle) {
        swizzle = Swizzle::XYZW;
    }

    Variants image_options = options.clone_except("usage");
    image_options.set("swizzle", swizzle);

    if (color) {
        image_options.set("color", color);
    }

    uint32_t const decoded = decode_name(filename);

    bool const is_id = resource::Null != decoded;

    uint32_t const image_id = is_id ? decoded : resources.load<Image>(filename, image_options).id;

    auto const image = resources.get<Image>(image_id);

    if (!image) {
        logging::error("Loading texture %S: ", filename);
        return Texture();
    }

    if (Image::Type::Byte1 == image->type()) {
        return Texture(Texture::Type::Byte1_unorm, image_id, scale);
    }

    if (Image::Type::Byte2 == image->type()) {
        if (Usage::Normal == usage) {
            return Texture(Texture::Type::Byte2_snorm, image_id, scale);
        }

        return Texture(Texture::Type::Byte2_unorm, image_id, scale);
    }

    if (Image::Type::Byte3 == image->type()) {
        if (Usage::Color == usage || Usage::Emission == usage) {
            return Texture(Texture::Type::Byte3_sRGB, image_id, scale);
        }

        return Texture(Texture::Type::Byte3_unorm, image_id, scale);
    }

    if (Image::Type::Byte4 == image->type()) {
        return Texture(Texture::Type::Byte4_sRGB, image_id, scale);
    }

    if (Image::Type::Short3 == image->type()) {
        return Texture(Texture::Type::Half3, image_id, scale);
    }

    if (Image::Type::Float1 == image->type()) {
        return Texture(Texture::Type::Float1, image_id, scale);
    }

    if (Image::Type::Float1_sparse == image->type()) {
        return Texture(Texture::Type::Float1_sparse, image_id, scale);
    }

    if (Image::Type::Float2 == image->type()) {
        return Texture(Texture::Type::Float2, image_id, scale);
    }

    if (Image::Type::Float3 == image->type()) {
        return Texture(Texture::Type::Float3, image_id, scale);
    }

    return Texture();
}

std::string Provider::encode_name(uint32_t image_id) {
    return "tex:" + std::to_string(image_id);
}

uint32_t Provider::decode_name(std::string_view name) {
    uint32_t id = resource::Null;

    if ("tex:" == name.substr(0, 4)) {
        std::from_chars(name.data() + 4, name.data() + name.size(), id);
    }

    return id;
}

}  // namespace image::texture
