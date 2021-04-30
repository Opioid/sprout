#include "average.hpp"
#include "base/math/print.hpp"
#include "base/string/string.hpp"
#include "core/image/texture/texture.inl"
#include "core/logging/logging.hpp"
#include "core/scene/scene.hpp"
#include "item.hpp"
#include "operator_helper.inl"

#include <sstream>

namespace op {

using namespace scene;

uint32_t average(std::vector<Item> const& items, it::options::Options const& /*options*/,
                 Scene const&             scene, Threads& /*threads*/) {
    for (auto const& i : items) {
        if (1 == i.image.num_channels()) {
            logging::info(string::to_string(round(i.image.average_1(scene), 4)));
        } else {
            float3 const a = i.image.average_3(scene);

            std::ostringstream stream;
            stream << float3(round(a[0], 4), round(a[1], 4), round(a[2], 2));

            logging::info(stream.str());
        }
    }

    return uint32_t(items.size());
}

}  // namespace op
