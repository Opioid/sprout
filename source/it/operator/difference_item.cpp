#include "difference_item.hpp"
#include "base/thread/thread_pool.hpp"
#include "core/image/texture/texture.inl"
#include "item.hpp"

namespace op {

static inline float round(float x, uint32_t d) noexcept {
    float const f = std::pow(10.f, static_cast<float>(d));
    return static_cast<float>(static_cast<uint32_t>(x * f + 0.5f)) / f;
}

Difference_item::Difference_item(Item const& item) noexcept
    : name_(item.name_out.empty() ? item.name.substr(0, item.name.find_last_of('.')) + "_dif.png"
                                  : item.name_out),
      image_(item.image) {
    int2 const d = item.image->dimensions_2();
    difference_  = new float[d[0] * d[1]];
}

Difference_item::~Difference_item() noexcept {
    delete[] difference_;
}

std::string Difference_item::name() const noexcept {
    return name_;
}

float const* Difference_item::difference() const noexcept {
    return difference_;
}

float Difference_item::max_dif() const noexcept {
    return round(max_dif_, 4);
}

float Difference_item::rmse() const noexcept {
    return round(rmse_, 4);
}

float Difference_item::psnr() const noexcept {
    return round(psnr_, 2);
}

void Difference_item::calculate_difference(Texture const* other, Scratch* scratch, float clamp,
                                           float2 clip, thread::Pool& pool) noexcept {
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

                dif = (dif < args.clip[0] || dif > args.clip[1]) ? 0.f : std::min(dif, args.clamp);

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

}  // namespace op