#include "concatenate.hpp"
#include <fstream>
#include "core/image/encoding/png/png_writer.hpp"
#include "core/image/texture/texture.inl"
#include "core/image/typed_image.hpp"
#include "core/logging/logging.hpp"
#include "item.hpp"

#include <iostream>
#include "base/math/print.hpp"

namespace op {

using namespace image;

int2 calculate_dimensions(std::vector<Item> const& items, uint32_t num_per_row) noexcept;

void copy(texture::Texture const& source, Float4& destination, int2 offset);

uint32_t concatenate(std::vector<Item> const& items, thread::Pool& pool) noexcept {
    uint32_t const num_per_row = 3u;

    int2 const dimensions = calculate_dimensions(items, num_per_row);

    std::cout << dimensions << std::endl;

    Float4 target = Float4(image::Description(dimensions));

    int2 offset(0);

    uint32_t column     = 0;
    int32_t  row_height = 0;

    for (auto const& i : items) {
        copy(*i.image, target, offset);

        int2 const d = i.image->dimensions_2();

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

    encoding::png::Writer writer(dimensions, false);

    std::ofstream stream("test.png", std::ios::binary);

    writer.write(stream, target, pool);

    return 0;
}

int2 calculate_dimensions(std::vector<Item> const& items, uint32_t num_per_row) noexcept {
    int2 dimensions(0);

    int32_t  row_width  = 0;
    int32_t  row_height = 0;
    uint32_t column     = 0;

    for (auto const& i : items) {
        int2 const d = i.image->dimensions_2();

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

void copy(texture::Texture const& source, Float4& destination, int2 offset) {
    int2 const sd = source.dimensions_2();

    for (int32_t y = 0; y < sd[1]; ++y) {
        for (int32_t x = 0; x < sd[0]; ++x) {
            destination.store(offset[0] + x, offset[1] + y, source.at_4(x, y, 0));
        }
    }
}

}  // namespace op
