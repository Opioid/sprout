#include "add.hpp"
#include "base/math/print.hpp"
#include "core/image/texture/texture.inl"
#include "core/logging/logging.hpp"
#include "item.hpp"
#include "operator_helper.hpp"

namespace op {

uint32_t add(std::vector<Item> const& items, it::options::Options const& /*options*/,
             thread::Pool&            threads) {
    int2 const d = max_dimensions_2(items);

    Float4 target = Float4(image::Description(d));

    target.clear(float4(0.f));

    for (int32_t y = 0; y < d[1]; ++y) {
        for (int32_t x = 0; x < d[0]; ++x) {
            float4 a = target.at(x, y);

            for (auto const& i : items) {
                int2 const db = i.image->dimensions_2();

                if ((x >= db[0]) | (y >= db[1])) {
                    continue;
                }

                float4 const b = i.image->at_4(x, y);

                a += b;
            }

            target.store(x, y, a);
        }
    }

    bool const alpha = any_has_alpha_channel(items);

    std::string const name = name_out(items, "add");

    write(target, name, alpha, threads);

    return uint32_t(items.size());
}

uint32_t sub(std::vector<Item> const& items, it::options::Options const& /*options*/,
             thread::Pool&            threads) {
    int2 const d = items[0].image->dimensions_2();

    Float4 target = Float4(image::Description(d));

    auto const image = items[0].image;

    for (int32_t y = 0; y < d[1]; ++y) {
        for (int32_t x = 0; x < d[0]; ++x) {
            float4 a = image->at_4(x, y);

            for (auto const& i : items) {
                int2 const db = i.image->dimensions_2();

                if ((x >= db[0]) | (y >= db[1])) {
                    continue;
                }

                float4 const b = i.image->at_4(x, y);

                a = float4(max(a.xyz() - b.xyz(), 0.f), a[3]);
            }

            target.store(x, y, a);
        }
    }

    bool const alpha = any_has_alpha_channel(items);

    std::string const name = name_out(items, "sub");

    write(target, name, alpha, threads);

    return uint32_t(items.size());
}

}  // namespace op
