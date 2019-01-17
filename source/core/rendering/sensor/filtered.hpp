#ifndef SU_CORE_RENDERING_SENSOR_FILTERED_HPP
#define SU_CORE_RENDERING_SENSOR_FILTERED_HPP

#include <memory>
#include "base/math/vector.hpp"

namespace image::texture {
class Texture;
}

namespace sampler {
struct Camera_sample;
}

namespace rendering::sensor {

namespace filter {
class Filter;
}

template <class Base, class Clamp>
class Filtered : public Base {
  public:
    Filtered(int2 dimensions, float exposure, const Clamp& clamp,
             filter::Filter const* filter) noexcept;

    Filtered(int2 dimensions, float exposure, image::texture::Texture* backplate,
             const Clamp& clamp, filter::Filter const* filter) noexcept;

    ~Filtered() noexcept override;

    int32_t filter_radius_int() const noexcept override final;

    int4 isolated_tile(int4 const& tile) const noexcept override final;

  protected:
    void add_weighted(int2 pixel, float weight, float4 const& color, int4 const& isolated,
                      int4 const& bounds) noexcept;

    void weight_and_add(int2 pixel, float2 relative_offset, float4 const& color,
                        int4 const& isolated, int4 const& bounds) noexcept;

    Clamp clamp_;

    filter::Filter const* filter_;
};

template <class Base, class Clamp>
class Filtered_1p0 : public Filtered<Base, Clamp> {
  public:
    Filtered_1p0(int2 dimensions, float exposure, const Clamp& clamp,
                 filter::Filter const* filter) noexcept;

    Filtered_1p0(int2 dimensions, float exposure, image::texture::Texture* backplate,
                 const Clamp& clamp, filter::Filter const* filter) noexcept;

    void add_sample(sampler::Camera_sample const& sample, float4 const&, int4 const& isolated,
                    int4 const& bounds) noexcept override final;
};

template <class Base, class Clamp>
class Filtered_2p0 : public Filtered<Base, Clamp> {
  public:
    Filtered_2p0(int2 dimensions, float exposure, const Clamp& clamp,
                 filter::Filter const* filter) noexcept;

    Filtered_2p0(int2 dimensions, float exposure, image::texture::Texture* backplate,
                 const Clamp& clamp, filter::Filter const* filter) noexcept;

    void add_sample(sampler::Camera_sample const& sample, float4 const&, int4 const& isolated,
                    int4 const& bounds) noexcept override final;
};

template <class Base, class Clamp>
class Filtered_inf : public Filtered<Base, Clamp> {
  public:
    Filtered_inf(int2 dimensions, float exposure, const Clamp& clamp,
                 filter::Filter const* filter) noexcept;

    Filtered_inf(int2 dimensions, float exposure, image::texture::Texture* backplate,
                 const Clamp& clamp, filter::Filter const* filter) noexcept;

    void add_sample(sampler::Camera_sample const& sample, float4 const&, int4 const& isolated,
                    int4 const& bounds) noexcept override final;
};

}  // namespace rendering::sensor

#endif
