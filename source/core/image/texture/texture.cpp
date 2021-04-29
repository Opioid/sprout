#include "texture.hpp"
#include "texture.inl"

namespace image::texture {

float Turbotexture::at_1(int32_t x, int32_t y, Scene const& scene) const {
    Image const* image = scene.image(image_id_);

    switch (type_) {
    case Type::Byte1_unorm: {
        uint8_t const value = image->byte1().at(x, y);
        return encoding::cached_unorm_to_float(value);
    }
    }

    return 0.f;
}

float2 Turbotexture::at_2(int32_t x, int32_t y, Scene const& scene) const {
    Image const* image = scene.image(image_id_);

    switch (type_) {
    case Type::Byte2_unorm: {
        byte2 const value = image->byte2().at(x, y);
        return encoding::cached_unorm_to_float(value);
    }
    }

    return float2(0.f);
}

float3 Turbotexture::at_3(int32_t x, int32_t y, Scene const& scene) const {
    Image const* image = scene.image(image_id_);

    switch (type_) {
    case Type::Byte3_snorm: {
        byte3 const value = image->byte3().at(x, y);
        return encoding::cached_snorm_to_float(value);
    }
    case Type::Byte3_sRGB: {
        byte3 const value = image->byte3().at(x, y);
#ifdef SU_ACESCG
    return spectrum::sRGB_to_AP1(encoding::cached_srgb_to_float(value));
#else
    return encoding::cached_srgb_to_float(value);
#endif
    }
    case Type::Float3: {
        return float3(image->float3().at(x, y));
    }
    }

    return float3(0.f);
}

float4 Turbotexture::at_4(int32_t x, int32_t y, Scene const& scene) const {
    return float4(0.f);
}

void Turbotexture::gather_1(int4_p xy_xy1, Scene const& scene, float c[4]) const {
    Image const* image = scene.image(image_id_);

    switch (type_) {
    case Type::Byte1_unorm: {
        uint8_t values[4];
        image->byte1().gather(xy_xy1, values);

    c[0] = encoding::cached_unorm_to_float(values[0]);
    c[1] = encoding::cached_unorm_to_float(values[1]);
    c[2] = encoding::cached_unorm_to_float(values[2]);
    c[3] = encoding::cached_unorm_to_float(values[3]);

    }
    }
}

void Turbotexture::gather_2(int4_p xy_xy1, Scene const& scene, float2 c[4]) const {
    Image const* image = scene.image(image_id_);

    switch (type_) {
    case Type::Byte2_unorm: {
        byte2 values[4];
        image->byte2().gather(xy_xy1, values);

    c[0] = encoding::cached_unorm_to_float(values[0]);
    c[1] = encoding::cached_unorm_to_float(values[1]);
    c[2] = encoding::cached_unorm_to_float(values[2]);
    c[3] = encoding::cached_unorm_to_float(values[3]);

    }
    }

}

void Turbotexture::gather_3(int4_p xy_xy1, Scene const& scene, float3 c[4]) const {
    Image const* image = scene.image(image_id_);

    switch (type_) {
    case Type::Byte3_snorm: {
        byte3 values[4];
        image->byte3().gather(xy_xy1, values);

    c[0] = encoding::cached_snorm_to_float(values[0]);
    c[1] = encoding::cached_snorm_to_float(values[1]);
    c[2] = encoding::cached_snorm_to_float(values[2]);
    c[3] = encoding::cached_snorm_to_float(values[3]);
    break;
    }

    case Type::Byte3_sRGB: {
        byte3 values[4];
        image->byte3().gather(xy_xy1, values);
#ifdef SU_ACESCG
    c[0] = spectrum::sRGB_to_AP1(encoding::cached_srgb_to_float(values[0]));
    c[1] = spectrum::sRGB_to_AP1(encoding::cached_srgb_to_float(values[1]));
    c[2] = spectrum::sRGB_to_AP1(encoding::cached_srgb_to_float(values[2]));
    c[3] = spectrum::sRGB_to_AP1(encoding::cached_srgb_to_float(values[3]));
#else
    c[0] = encoding::cached_srgb_to_float(values[0]);
    c[1] = encoding::cached_srgb_to_float(values[1]);
    c[2] = encoding::cached_srgb_to_float(values[2]);
    c[3] = encoding::cached_srgb_to_float(values[3]);
#endif
        break;
    }
    case Type::Float3: {
        packed_float3 values[4];
        image->float3().gather(xy_xy1, values);
        c[0] = float3(values[0]);
        c[1] = float3(values[1]);
        c[2] = float3(values[2]);
        c[3] = float3(values[3]);
        break;
    }
    }

}

float Turbotexture::at_element_1(int32_t x, int32_t y, int32_t element, Scene const& scene) const {
    return 0.f;
}

float2 Turbotexture::at_element_2(int32_t x, int32_t y, int32_t element, Scene const& scene) const {
    return float2(0.f);
}

float3 Turbotexture::at_element_3(int32_t x, int32_t y, int32_t element, Scene const& scene) const {
    return float3(0.f);
}

float Turbotexture::at_1(int32_t x, int32_t y, int32_t z, Scene const& scene) const {
    Image const* image = scene.image(image_id_);

    switch (type_) {
    case Type::Float1: {
        return image->float1().at(x, y, z);
    }
    case Type::Float1_sparse: {
        return image->float1_sparse().at(x, y, z);
    }
    case Type::Float2: {
        return image->float2().at(x, y, z)[0];
    }
    }

    return 0.f;
}

float2 Turbotexture::at_2(int32_t x, int32_t y, int32_t z, Scene const& scene) const {
    Image const* image = scene.image(image_id_);

    switch (type_) {
    case Type::Float2: {
        return image->float2().at(x, y, z);
    }
    }

    return float2(0.f);
}

float3 Turbotexture::at_3(int32_t x, int32_t y, int32_t z, Scene const& scene) const {
    return float3(0.f);
}

float4 Turbotexture::at_4(int32_t x, int32_t y, int32_t z, Scene const& scene) const {
    return float4(0.f);
}

void Turbotexture::gather_1(int3_p xyz, int3_p xyz1, Scene const& scene, float c[8]) const {
    Image const* image = scene.image(image_id_);

    switch (type_) {
    case Type::Float1: {
        image->float1().gather(xyz, xyz1, c);
        break;
    }
    case Type::Float1_sparse: {
        image->float1_sparse().gather(xyz, xyz1, c);
        break;
    }
    case Type::Float2: {
        float2 values[8];
        image->float2().gather(xyz, xyz1, values);
        c[0] = values[0][0];
        c[1] = values[1][0];
        c[2] = values[2][0];
        c[3] = values[3][0];
        c[4] = values[4][0];
        c[5] = values[5][0];
        c[6] = values[6][0];
        c[7] = values[7][0];
        break;
    }
    }
}

void Turbotexture::gather_2(int3_p xyz, int3_p xyz1, Scene const& scene, float2 c[8]) const {
    Image const* image = scene.image(image_id_);

    switch (type_) {
    case Type::Float2: {
        image->float2().gather(xyz, xyz1, c);
        break;
    }
    }
}

float Turbotexture::average_1(Scene const& scene) const {
    float average(0.f);

    auto const d = description(scene).dimensions();
    for (int32_t z = 0, depth = d[2]; z < depth; ++z) {
        for (int32_t y = 0, height = d[1]; y < height; ++y) {
            for (int32_t x = 0, width = d[0]; x < width; ++x) {
                average += at_1(x, y, z, scene);
            }
        }
    }

    auto const df = float3(d);
    return average / (df[0] * df[1] * d[2]);
}

float3 Turbotexture::average_3(Scene const& scene) const {
    float3 average(0.f);

    auto const d = description(scene).dimensions().xy();
    for (int32_t y = 0; y < d[1]; ++y) {
        for (int32_t x = 0; x < d[0]; ++x) {
            average += at_3(x, y, scene);
        }
    }

    auto const df = float2(d);
    return average / (df[0] * df[1]);
}

}  // namespace image::texture
