#include "difference.hpp"
#include "base/memory/array.inl"
#include "base/thread/thread_pool.hpp"
#include "core/image/encoding/png/png_writer.hpp"
#include "core/image/texture/texture.inl"
#include "core/logging/logging.hpp"
#include "item.hpp"

using namespace image;

using Texture = texture::Texture;

namespace op {

class Candidate {
  public:
    Candidate(Item const& item) noexcept
        : name_(item.name.substr(0, item.name.find_last_of('.')) + "_dif.png"), image_(item.image) {
        int2 const d = item.image->dimensions_2();
        difference_  = new float[d[0] * d[1]];
    }

    ~Candidate() noexcept {
        delete[] difference_;
    }

    std::string name() const noexcept {
        return name_;
    }

    float const* difference() const noexcept {
        return difference_;
    }

    float calculate_difference(Texture const* other, float* max_difs, float clamp, float clip,
                               thread::Pool& pool) noexcept {
        int2 const d = image_->dimensions_2();

        int32_t const num_pixel = d[0] * d[1];

        struct Args {
            Texture const* image;
            Texture const* other;

            float* difference;
            float* max_difs;

            float clamp;
            float clip;
        };

        Args args = Args{image_, other, difference_, max_difs, clamp, clip};

        pool.run_range(
            [&args](uint32_t id, int32_t begin, int32_t end) {
                float max_dif = 0.f;

                for (int32_t i = begin; i < end; ++i) {
                    float3 const va = args.image->at_3(i);
                    float3 const vb = args.other->at_3(i);

                    float dif = distance(va, vb);

                    dif = dif > args.clip ? 0.f : std::min(dif, args.clamp);

                    args.difference[i] = dif;

                    max_dif = std::max(max_dif, dif);
                }

                args.max_difs[id] = max_dif;
            },
            0, num_pixel);

        float max_dif = max_difs[0];
        for (uint32_t i = 1, len = pool.num_threads(); i < len; ++i) {
            max_dif = std::max(max_dif, max_difs[i]);
        }

        return max_dif;
    }

  private:
    std::string name_;

    Texture const* image_;

    float* difference_;
};

uint32_t difference(std::vector<Item> const& items, float clamp, float clip, thread::Pool& pool) noexcept {
    if (items.size() < 2) {
        logging::error("Need at least 2 images for diff.");
        return 0;
    }

    Texture const* reference  = items[0].image;
    int2 const     dimensions = reference->dimensions_2();

    std::vector<Candidate> candidates;
    candidates.reserve(items.size() - 1);

    for (size_t i = 1, len = items.size(); i < len; ++i) {
        Item const& item = items[i];

        if (item.image->dimensions_2() != dimensions) {
            logging::error("%S does not match reference resolution", item.name);
            continue;
        }

        candidates.emplace_back(item);
    }

    memory::Array<float> max_difs(pool.num_threads(), 0.f);

    float max_dif = 0.f;

    for (auto& c : candidates) {
        max_dif = std::max(c.calculate_difference(reference, max_difs.data(), clamp, clip, pool),
                           max_dif);
    }

    encoding::png::Writer writer(dimensions, false);

    for (auto const& c : candidates) {
        writer.write_heatmap(c.name(), c.difference(), dimensions, max_dif);
    }

    return static_cast<uint32_t>(candidates.size()) + 1;
}

}  // namespace op
