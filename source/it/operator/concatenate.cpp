#include "concatenate.hpp"
#include "core/image/encoding/png/png_writer.hpp"
#include "core/image/texture/texture.inl"
#include "core/logging/logging.hpp"
#include "item.hpp"

#include "base/math/print.hpp"
#include <iostream>

namespace op {

int2 calculate_dimensions(std::vector<Item> const& items, uint32_t num_per_row) noexcept;

uint32_t concatenate(std::vector<Item> const& items, thread::Pool& /*pool*/) noexcept {
    uint32_t const num_per_row = 1u;

    std::cout << calculate_dimensions(items, num_per_row) << std::endl;

    return 0;
}

int2 calculate_dimensions(std::vector<Item> const& items, uint32_t num_per_row) noexcept {
    int2 dimensions(0);

    int32_t row_width = 0;
    int32_t row_height = 0;
    uint32_t column = 0;

    for (auto const& i : items) {
        int2 const d = i.image->dimensions_2();

        row_width += d[0];
        row_height = std::max(row_height, d[1]);

        if (column >= num_per_row - 1) {
            dimensions[0] = std::max(dimensions[0], row_width);
            dimensions[1] += row_height;

            row_width = 0;
            row_height = 0;
            column = 0;
        } else {
            ++column;
        }
    }

    dimensions[0] = std::max(dimensions[0], row_width);
    dimensions[1] += row_height;

    return dimensions;
}

}
