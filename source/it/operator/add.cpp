#include "add.hpp"
#include "base/math/print.hpp"
#include "base/thread/thread_pool.hpp"
#include "core/image/texture/texture.inl"
#include "core/logging/logging.hpp"
#include "core/scene/scene.hpp"
#include "item.hpp"
#include "operator_helper.hpp"

namespace op {

using namespace scene;
using namespace it::options;

uint32_t add(Items const& items, Options const& /*options*/, Scene const& scene, Threads& threads) {
    int2 const d = max_dimensions_2(items, scene);

    Float4 target = Float4(image::Description(d));

    threads.run_range(
        [&items, &target, &scene](uint32_t /*id*/, int32_t begin, int32_t end) noexcept {
            int2 const d = target.description().dimensions().xy();

            for (int32_t y = begin; y < end; ++y) {
                for (int32_t x = 0; x < d[0]; ++x) {
                    float4 a(0.f);

                    for (auto const& i : items) {
                        int2 const db = i.image.description(scene).dimensions().xy();

                        if ((x >= db[0]) | (y >= db[1])) {
                            continue;
                        }

                        float4 const b = i.image.at_4(x, y, scene);

                        a += b;
                    }

                    target.store(x, y, a);
                }
            }
        },
        0, d[1]);

    bool const alpha = any_has_alpha_channel(items);

    std::string const name = name_out(items, "add");

    write(target, name, alpha, threads);

    return uint32_t(items.size());
}

uint32_t sub(Items const& items, Options const& /*options*/, Scene const& scene, Threads& threads) {
    int2 const d = items[0].image.description(scene).dimensions().xy();

    Float4 target = Float4(image::Description(d));

    threads.run_range(
        [&items, &scene, &target](uint32_t /*id*/, int32_t begin, int32_t end) noexcept {
            auto const& image0 = items[0].image;

            int2 const d = image0.description(scene).dimensions().xy();

            for (int32_t y = begin; y < end; ++y) {
                for (int32_t x = 0; x < d[0]; ++x) {
                    float4 a = image0.at_4(x, y, scene);

                    for (size_t i = 1, len = items.size(); i < len; ++i) {
                        auto const& imagei = items[i].image;

                        int2 const db = imagei.description(scene).dimensions().xy();

                        if ((x >= db[0]) | (y >= db[1])) {
                            continue;
                        }

                        float4 const b = imagei.at_4(x, y, scene);

                        a = float4(max(a.xyz() - b.xyz(), 0.f), a[3]);
                    }

                    target.store(x, y, a);
                }
            }
        },
        0, d[1]);

    bool const alpha = any_has_alpha_channel(items);

    std::string const name = name_out(items, "sub");

    write(target, name, alpha, threads);

    return uint32_t(items.size());
}

}  // namespace op
