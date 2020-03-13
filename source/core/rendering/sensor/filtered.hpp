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
    Filtered(Clamp const& clamp, F&& filter);

    ~Filtered() override;

    int32_t filter_radius_int() const final;

    int4 isolated_tile(int4 const& tile) const final;

  protected:
    void add_weighted(int2 pixel, float weight, float4 const& color, int4 const& isolated,
                      int4 const& bounds);

    void add_weighted(int2 pixel, float weight, float4 const& color, int4 const& bounds);

    void weight_and_add(int2 pixel, float2 relative_offset, float4 const& color,
                        int4 const& isolated, int4 const& bounds);

    void weight_and_add(int2 pixel, float2 relative_offset, float4 const& color,
                        int4 const& bounds);

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

    Filtered_1p0(Clamp const& clamp, F&& filter);

    void add_sample(Sample const& sample, float4 const&, int4 const& isolated, int2 offset,
                    int4 const& bounds) final;

    void splat_sample(Sample_to const& sample, float4 const& color, int2 offset,
                      int4 const& bounds) final;
};

template <class Base, class Clamp, class F>
class Filtered_2p0 final : public Filtered<Base, Clamp, F> {
  public:
    using Sample        = sampler::Camera_sample;
    using Sample_to     = sampler::Camera_sample_to;
    using Filter        = filter::Filter;
    using Filtered_base = Filtered<Base, Clamp, F>;

    Filtered_2p0(Clamp const& clamp, F&& filter);

    void add_sample(Sample const& sample, float4 const&, int4 const& isolated, int2 offset,
                    int4 const& bounds) final;

    void splat_sample(Sample_to const& sample, float4 const& color, int2 offset,
                      int4 const& bounds) final;
};

template <class Base, class Clamp, class F>
class Filtered_inf final : public Filtered<Base, Clamp, F> {
  public:
    using Sample        = sampler::Camera_sample;
    using Sample_to     = sampler::Camera_sample_to;
    using Filter        = filter::Filter;
    using Filtered_base = Filtered<Base, Clamp, F>;

    Filtered_inf(Clamp const& clamp, F&& filter);

    void add_sample(Sample const& sample, float4 const&, int4 const& isolated, int2 offset,
                    int4 const& bounds) final;

    void splat_sample(Sample_to const& sample, float4 const& color, int2 offset,
                      int4 const& bounds) final;
};

}  // namespace rendering::sensor

#endif
