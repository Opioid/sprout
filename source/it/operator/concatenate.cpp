#include "concatenate.hpp"
#include "core/image/texture/texture.inl"
#include "core/image/typed_image.hpp"
#include "core/logging/logging.hpp"
#include "core/scene/camera/camera_perspective.hpp"
#include "core/scene/scene.hpp"
#include "item.hpp"
#include "operator_helper.hpp"
#include "options/options.hpp"
#include "rendering/postprocessor/postprocessor_pipeline.hpp"

namespace op {

Pipeline::~Pipeline() {
    delete camera;
}

using namespace image;
using namespace scene;

int2 calculate_dimensions(std::vector<Item> const& items, uint32_t num_per_row, Scene const& scene);

void copy(texture::Texture const& source, Float4& destination, int2 offset, float2 clip,
          Scene const& scene);

uint32_t concatenate(std::vector<Item> const& items, it::options::Options const& options,
                     Pipeline& pipeline, Scene const& scene, Threads& threads) {
    uint32_t num_per_row = options.concat_num_per_row;

    float2 const clip = options.clip;

    bool const alpha = any_has_alpha_channel(items);

    if (0 == num_per_row) {
        num_per_row = 0xFFFFFFFF;
    }

    int2 const dimensions = calculate_dimensions(items, num_per_row, scene);

    Float4 target = Float4(image::Description(dimensions));

    int2 offset(0);

    uint32_t column     = 0;
    int32_t  row_height = 0;

    for (auto const& i : items) {
        copy(i.image, target, offset, clip, scene);

        int2 const d = i.image.description(scene).dimensions().xy();

        offset[0] += d[0];

        row_height = std::max(row_height, d[1]);

        ++column;

        if (column >= num_per_row) {
            offset[0] = 0;
            offset[1] += row_height;

            column     = 0;
            row_height = 0;
        }
    }

    if (!pipeline.pp.empty()) {
        if (pipeline.camera) {
            pipeline.pp.init(*pipeline.camera, threads);
        } else {
            scene::camera::Perspective camera;

            camera.set_resolution(dimensions, int4(int2(0), dimensions));

            pipeline.pp.init(camera, threads);
        }

        pipeline.pp.apply(target, scene, threads);
    }

    std::string const name = name_out(items, "concat");

    write(target, name, alpha, threads);

    return uint32_t(items.size());
}

int2 calculate_dimensions(std::vector<Item> const& items, uint32_t num_per_row,
                          Scene const& scene) {
    int2 dimensions(0);

    int32_t  row_width  = 0;
    int32_t  row_height = 0;
    uint32_t column     = 0;

    for (auto const& i : items) {
        int2 const d = i.image.description(scene).dimensions().xy();

        row_width += d[0];
        row_height = std::max(row_height, d[1]);

        ++column;

        if (column >= num_per_row) {
            dimensions[0] = std::max(dimensions[0], row_width);
            dimensions[1] += row_height;

            row_width  = 0;
            row_height = 0;
            column     = 0;
        }
    }

    dimensions[0] = std::max(dimensions[0], row_width);
    dimensions[1] += row_height;

    return dimensions;
}

static inline float4 clip(float4_p v, float2 clip) {
    return float4((v[0] < clip[0] || v[0] > clip[1]) ? 0.f : v[0],
                  (v[1] < clip[0] || v[1] > clip[1]) ? 0.f : v[1],
                  (v[2] < clip[0] || v[2] > clip[1]) ? 0.f : v[2], v[3]);
}

void copy(Texture const& source, Float4& destination, int2 offset, float2 clip,
          Scene const& scene) {
    int2 const sd = source.description(scene).dimensions().xy();

    for (int32_t y = 0; y < sd[1]; ++y) {
        for (int32_t x = 0; x < sd[0]; ++x) {
            float4 const s = source.at_4(x, y, scene);

            float4 const d = op::clip(s, clip);

            destination.store(offset[0] + x, offset[1] + y, d);
        }
    }
}

}  // namespace op
