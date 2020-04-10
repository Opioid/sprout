#include "average.hpp"
#include "base/math/print.hpp"
#include "base/string/string.hpp"
#include "core/image/texture/texture.inl"
#include "core/logging/logging.hpp"
#include "item.hpp"
#include "operator_helper.inl"

#include <sstream>

namespace op {
uint32_t average(std::vector<Item> const& items, it::options::Options const& /*options*/,
                 thread::Pool& /*threads*/) {
    for (auto const& i : items) {
        if (1 == i.image->num_channels()) {
            logging::info(string::to_string(round(i.image->average_1(), 4)));
        } else {
            float3 const a = i.image->average_3();

            std::ostringstream stream;
            stream << float3(round(a[0], 4), round(a[1], 4), round(a[2], 2));

            logging::info(stream.str());
        }
    }

    return uint32_t(items.size());
}

}  // namespace op
