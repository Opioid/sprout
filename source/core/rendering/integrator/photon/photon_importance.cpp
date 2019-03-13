#include "photon_importance.hpp"
#include "base/atomic/atomic.hpp"
#include "base/math/distribution/distribution_1d.inl"
#include "base/math/vector2.inl"
#include "base/memory/align.hpp"
#include "base/thread/thread_pool.hpp"
#include "image/encoding/png/png_writer.hpp"

namespace rendering::integrator::photon {

static uint32_t constexpr Dimensions = 256;

Importance::Importance() noexcept
    : dimensions_(Dimensions),
      importance_(memory::allocate_aligned<uint32_t>(Dimensions * Dimensions)),
      dimensions_back_(Dimensions - 1),
      dimensions_float_(float2(Dimensions)) {
    for (int32_t i = 0, len = dimensions_[i] * dimensions_[1]; i < len; ++i) {
        importance_[i] = 0;
    }
}

Importance::~Importance() noexcept {
    memory::free_aligned(importance_);
}

void Importance::increment(float2 uv) noexcept {
    int32_t const x = std::min(static_cast<int32_t>(uv[0] * dimensions_float_[0]),
                               dimensions_back_[0]);
    int32_t const y = std::min(static_cast<int32_t>(uv[1] * dimensions_float_[1]),
                               dimensions_back_[1]);

    int32_t const id = y * dimensions_[0] + x;

    atomic::add_assign(importance_[id], 1);
}

void Importance::export_heatmap(std::string_view name) const noexcept {
    image::encoding::png::Writer::write_heatmap(name, importance_, dimensions_);
}

void Importance::prepare_sampling(thread::Pool& pool) noexcept {
    Distribution_2D::Distribution_impl* conditional = distribution_.allocate(Dimensions);
    /*
            pool.run_range(
                    [&conditional](
                            uint32_t id, int32_t begin, int32_t end) {
                            float* luminance = memory::allocate_aligned<float>(d[0]);

                            float4 artw(0.f);

                            for (int32_t y = begin; y < end; ++y) {
                                    float const v = idf[1] * (static_cast<float>(y) + 0.5f);

                                    for (int32_t x = 0; x < d[0]; ++x) {
                                            float const u = idf[0] * (static_cast<float>(x) + 0.5f);

                                            float const uv_weight = shape.uv_weight(float2(u, v));

                                            float3 const radiance = ef * texture.at_element_3(x, y,
       element);

                                            float3 const wr = uv_weight * radiance;

                                            artw += float4(wr, uv_weight);

                                            luminance[x] = spectrum::luminance(wr);
                                    }

                                    conditional[y].init(luminance, d[0]);
                            }

                            artws[id] += artw;

                            memory::free_aligned(luminance);
                    },
                    0, d[1]);

            float4 artw(0.f);
            for (auto const& a : artws) {
                    artw += a;
            }

            average_emission_ = artw.xyz() / artw[3];

            total_weight_ = artw[3];
    */
    distribution_.init();
}

}  // namespace rendering::integrator::photon
