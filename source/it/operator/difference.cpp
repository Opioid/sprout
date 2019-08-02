#include "difference.hpp"
#include "base/memory/array.inl"
#include "base/string/string.hpp"
#include "base/thread/thread_pool.hpp"
#include "core/image/encoding/png/png_writer.hpp"
#include "core/image/texture/texture.inl"
#include "core/logging/logging.hpp"
#include "item.hpp"

using namespace image;

using Texture = texture::Texture;

namespace op {

struct Scratch {
    float max_val;
    float max_dif;
    float dif_sum;
};

static inline float round(float x, uint32_t d) noexcept {
    float const f = std::pow(10.f, static_cast<float>(d));
    return static_cast<float>(static_cast<uint32_t>(x * f + 0.5f)) / f;
}

class Candidate {
  public:
    Candidate(Item const& item) noexcept
        : name_(item.name_out.empty()
                    ? item.name.substr(0, item.name.find_last_of('.')) + "_dif.png"
                    : item.name_out),
          image_(item.image) {
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

    float max_dif() const noexcept {
        return max_dif_;
    }

    float psnr() const noexcept {
        return psnr_;
    }

    float rmse() const noexcept {
        return rmse_;
    }

    void calculate_difference(Texture const* other, Scratch* scratch, float clamp, float2 clip,
                              thread::Pool& pool) noexcept {
        int2 const d = image_->dimensions_2();

        int32_t const num_pixel = d[0] * d[1];

        struct Args {
            Texture const* image;
            Texture const* other;

            float* difference;

            Scratch* scratch;

            float  clamp;
            float2 clip;
        };

        Args args = Args{image_, other, difference_, scratch, clamp, clip};

        pool.run_range(
            [&args](uint32_t id, int32_t begin, int32_t end) {
                float max_val = 0.f;
                float max_dif = 0.f;
                float dif_sum = 0.f;

                for (int32_t i = begin; i < end; ++i) {
                    float3 const va = args.image->at_3(i);
                    float3 const vb = args.other->at_3(i);

                    max_val = std::max(max_val, max_component(va));

                    float dif = distance(va, vb);

                    dif = (dif < args.clip[0] || dif > args.clip[1]) ? 0.f
                                                                     : std::min(dif, args.clamp);

                    args.difference[i] = dif;

                    max_dif = std::max(max_dif, dif);

                    dif_sum += dif * dif;
                }

                args.scratch[id].max_val = max_val;
                args.scratch[id].max_dif = max_dif;
                args.scratch[id].dif_sum = dif_sum;
            },
            0, num_pixel);

        float max_val = args.scratch[0].max_val;
        float max_dif = args.scratch[0].max_dif;
        float dif_sum = args.scratch[0].dif_sum;

        for (uint32_t i = 1, len = pool.num_threads(); i < len; ++i) {
            max_val = std::max(max_val, args.scratch[i].max_val);

            max_dif = std::max(max_dif, args.scratch[i].max_dif);

            dif_sum += args.scratch[i].dif_sum;
        }

        max_dif_ = max_dif;

        // * 3 because we compare the difference over 3 channels
        float const rmse = std::sqrt(dif_sum / static_cast<float>(3 * num_pixel));

        psnr_ = -20.f * std::log10(rmse / max_val);

        rmse_ = rmse;
    }

  private:
    std::string name_;

    Texture const* image_;

    float* difference_;

    float max_dif_;

    float psnr_;
    float rmse_;
};

uint32_t difference(std::vector<Item> const& items, float clamp, float2 clip,
                    thread::Pool& pool) noexcept {
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

    memory::Array<Scratch> scratch(pool.num_threads(), Scratch{0.f, 0.f, 0.f});

    float max_dif = 0.f;

    for (auto& c : candidates) {
        c.calculate_difference(reference, scratch.data(), clamp, clip, pool);

        max_dif = std::max(c.max_dif(), max_dif);

        logging::info("%S \n RMSE: " + string::to_string(round(c.rmse(), 4)) +
                          "\n PSNR: " + string::to_string(round(c.psnr(), 2)) + " dB",
                      c.name());
    }

    encoding::png::Writer writer(dimensions, false);

    for (auto const& c : candidates) {
        writer.write_heatmap(c.name(), c.difference(), dimensions, max_dif);
    }

    return static_cast<uint32_t>(candidates.size()) + 1;
}

}  // namespace op
