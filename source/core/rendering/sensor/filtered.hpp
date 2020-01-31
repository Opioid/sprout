#ifndef SU_CORE_RENDERING_SENSOR_FILTERED_HPP
#define SU_CORE_RENDERING_SENSOR_FILTERED_HPP

#include "base/math/vector.hpp"

namespace sampler {
struct Camera_sample;
struct Camera_sample_to;
}  // namespace sampler

namespace rendering::sensor {

namespace filter {
class Filter;
}

template <class Base, class Clamp, class F>
class Filtered : public Base {
  public:
    Filtered(float exposure, Clamp const& clamp, F&& filter) noexcept;

    ~Filtered() noexcept override;

    int32_t filter_radius_int() const noexcept final;

    int4 isolated_tile(int4 const& tile) const noexcept final;

  protected:
    void add_weighted(int2 pixel, float weight, float4 const& color, int4 const& isolated,
                      int4 const& bounds) noexcept;

    void add_weighted(int2 pixel, float weight, float4 const& color, int4 const& bounds) noexcept;

    void weight_and_add(int2 pixel, float2 relative_offset, float4 const& color,
                        int4 const& isolated, int4 const& bounds) noexcept;

    void weight_and_add(int2 pixel, float2 relative_offset, float4 const& color,
                        int4 const& bounds) noexcept;

    Clamp clamp_;

    F const filter_;
};

template <class Base, class Clamp, class F>
class Filtered_1p0 final : public Filtered<Base, Clamp, F> {
  public:
    using Sample        = sampler::Camera_sample;
    using Sample_to     = sampler::Camera_sample_to;
    using Filter        = filter::Filter;
    using Filtered_base = Filtered<Base, Clamp, F>;

    Filtered_1p0(float exposure, Clamp const& clamp, F&& filter) noexcept;

    void add_sample(Sample const& sample, float4 const&, int4 const& isolated,
                    int4 const& bounds) noexcept final;

    void splat_sample(Sample_to const& sample, float4 const& color,
                      int4 const& bounds) noexcept final;
};

template <class Base, class Clamp, class F>
class Filtered_2p0 final : public Filtered<Base, Clamp, F> {
  public:
    using Sample        = sampler::Camera_sample;
    using Sample_to     = sampler::Camera_sample_to;
    using Filter        = filter::Filter;
    using Filtered_base = Filtered<Base, Clamp, F>;

    Filtered_2p0(float exposure, Clamp const& clamp, F&& filter) noexcept;

    void add_sample(Sample const& sample, float4 const&, int4 const& isolated,
                    int4 const& bounds) noexcept final;

    void splat_sample(Sample_to const& sample, float4 const& color,
                      int4 const& bounds) noexcept final;
};

template <class Base, class Clamp, class F>
class Filtered_inf final : public Filtered<Base, Clamp, F> {
  public:
    using Sample        = sampler::Camera_sample;
    using Sample_to     = sampler::Camera_sample_to;
    using Filter        = filter::Filter;
    using Filtered_base = Filtered<Base, Clamp, F>;

    Filtered_inf(float exposure, Clamp const& clamp, F&& filter) noexcept;

    void add_sample(Sample const& sample, float4 const&, int4 const& isolated,
                    int4 const& bounds) noexcept final;

    void splat_sample(Sample_to const& sample, float4 const& color,
                      int4 const& bounds) noexcept final;
};

}  // namespace rendering::sensor

#endif
