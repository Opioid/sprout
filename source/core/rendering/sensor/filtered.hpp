#ifndef SU_CORE_RENDERING_SENSOR_FILTERED_HPP
#define SU_CORE_RENDERING_SENSOR_FILTERED_HPP

#include "base/math/vector.hpp"

namespace image::texture {
class Texture;
}

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
    using Filter  = filter::Filter;
    using Texture = image::texture::Texture;

    Filtered(int2 dimensions, float exposure, Clamp const& clamp, F&& filter) noexcept;

    Filtered(int2 dimensions, float exposure, Texture* backplate, Clamp const& clamp,
             F&& filter) noexcept;

    ~Filtered() noexcept override;

    int32_t filter_radius_int() const noexcept override final;

    int4 isolated_tile(int4 const& tile) const noexcept override final;

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
    using Camera_sample    = sampler::Camera_sample;
    using Camera_sample_to = sampler::Camera_sample_to;
    using Filter           = filter::Filter;
    using Texture          = image::texture::Texture;

    Filtered_1p0(int2 dimensions, float exposure, Clamp const& clamp, F&& filter) noexcept;

    Filtered_1p0(int2 dimensions, float exposure, Texture* backplate, Clamp const& clamp,
                 F&& filter) noexcept;

    void add_sample(Camera_sample const& sample, float4 const&, int4 const& isolated,
                    int4 const& bounds) noexcept override final;

    void splat_sample(Camera_sample_to const& sample, float4 const&,
                      int4 const&             bounds) noexcept override final;
};

template <class Base, class Clamp, class F>
class Filtered_2p0 final : public Filtered<Base, Clamp, F> {
  public:
    using Camera_sample    = sampler::Camera_sample;
    using Camera_sample_to = sampler::Camera_sample_to;
    using Filter           = filter::Filter;
    using Texture          = image::texture::Texture;

    Filtered_2p0(int2 dimensions, float exposure, Clamp const& clamp, F&& filter) noexcept;

    Filtered_2p0(int2 dimensions, float exposure, Texture* backplate, Clamp const& clamp,
                 F&& filter) noexcept;

    void add_sample(Camera_sample const& sample, float4 const&, int4 const& isolated,
                    int4 const& bounds) noexcept override final;

    void splat_sample(Camera_sample_to const& sample, float4 const&,
                      int4 const&             bounds) noexcept override final;
};

template <class Base, class Clamp, class F>
class Filtered_inf final : public Filtered<Base, Clamp, F> {
  public:
    using Camera_sample    = sampler::Camera_sample;
    using Camera_sample_to = sampler::Camera_sample_to;
    using Filter           = filter::Filter;
    using Texture          = image::texture::Texture;

    Filtered_inf(int2 dimensions, float exposure, Clamp const& clamp, F&& filter) noexcept;

    Filtered_inf(int2 dimensions, float exposure, Texture* backplate, Clamp const& clamp,
                 F&& filter) noexcept;

    void add_sample(Camera_sample const& sample, float4 const&, int4 const& isolated,
                    int4 const& bounds) noexcept override final;

    void splat_sample(Camera_sample_to const& sample, float4 const&,
                      int4 const&             bounds) noexcept override final;
};

}  // namespace rendering::sensor

#endif
