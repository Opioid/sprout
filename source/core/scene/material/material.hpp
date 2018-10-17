#ifndef SU_CORE_SCENE_MATERIAL_MATERIAL_HPP
#define SU_CORE_SCENE_MATERIAL_MATERIAL_HPP

#include <memory>
#include <vector>
#include "base/json/json_types.hpp"
#include "base/math/vector3.hpp"
#include "base/spectrum/discrete.hpp"
#include "collision_coefficients.hpp"
#include "image/texture/texture_adapter.hpp"
#include "image/texture/texture_types.hpp"
#include "sampler_settings.hpp"

namespace math {
struct Ray;
struct AABB;
}  // namespace math

namespace rnd {
class Generator;
}

namespace sampler {
class Sampler;
}

namespace thread {
class Pool;
}

namespace scene {

struct Renderstate;
class Worker;

namespace entity {
struct Composed_transformation;
}

namespace shape {
class Shape;
}

namespace material {

namespace volumetric {
class Gridtree;
}

class Sample;

class Material {
  public:
    using Sampler_filter = Sampler_settings::Filter;
    using Shape          = shape::Shape;
    using Transformation = entity::Composed_transformation;

    Material(Sampler_settings const& sampler_settings, bool two_sided) noexcept;

    virtual ~Material() noexcept;

    void set_mask(Texture_adapter const& mask) noexcept;

    void set_parameters(json::Value const& parameters) noexcept;

    virtual void compile() noexcept;

    virtual void tick(float absolute_time, float time_slice) noexcept;

    virtual const Sample& sample(float3 const& wo, Renderstate const& rs, Sampler_filter filter,
                                 sampler::Sampler& sampler, Worker const& worker,
                                 uint32_t depth) const noexcept = 0;

    virtual float3 evaluate_radiance(float3 const& wi, float2 uv, float area, uint64_t time,
                                     Sampler_filter filter, Worker const& worker) const noexcept;

    virtual float3 average_radiance(float area) const noexcept;

    virtual bool has_emission_map() const noexcept;

    struct Sample_2D {
        float2 uv;
        float  pdf;
    };
    virtual Sample_2D radiance_sample(float2 r2) const noexcept;

    virtual float emission_pdf(float2 uv, Sampler_filter filter, Worker const& worker) const
        noexcept;

    virtual float opacity(float2 uv, uint64_t time, Sampler_filter filter, Worker const& worker) const
        noexcept;

    virtual float3 thin_absorption(float3 const& wo, float3 const& n, float2 uv, uint64_t time,
                                   Sampler_filter filter, Worker const& worker) const noexcept;

    virtual float3 emission(math::Ray const& ray, Transformation const& transformation,
                            float step_size, rnd::Generator& rng, Sampler_filter filter,
                            Worker const& worker) const noexcept;

    virtual float3 absorption_coefficient(float2 uv, Sampler_filter filter,
                                          Worker const& worker) const noexcept;

    virtual CC collision_coefficients() const noexcept;

    virtual CC collision_coefficients(float2 uv, Sampler_filter filter, Worker const& worker) const
        noexcept;

    virtual CC collision_coefficients(float3 const& p, Sampler_filter filter,
                                      Worker const& worker) const noexcept;

    virtual CM control_medium() const noexcept;

    virtual volumetric::Gridtree const* volume_tree() const noexcept;

    virtual bool is_heterogeneous_volume() const noexcept;
    virtual bool is_textured_volume() const noexcept;
    virtual bool is_scattering_volume() const noexcept;

    virtual void prepare_sampling(Shape const& shape, uint32_t part,
                                  Transformation const& transformation, float area,
                                  bool importance_sampling, thread::Pool& pool) noexcept;

    virtual bool is_animated() const noexcept;

    virtual bool has_tinted_shadow() const noexcept;

    virtual float ior() const noexcept = 0;

    uint32_t sampler_key() const noexcept;

    virtual bool is_caustic() const noexcept;
    virtual bool is_masked() const noexcept;

    bool is_emissive() const noexcept;
    bool is_two_sided() const noexcept;

    virtual size_t num_bytes() const noexcept = 0;

  protected:
    virtual void set_parameter(std::string_view name, json::Value const& value) noexcept;

  private:
    uint32_t sampler_key_;

    bool two_sided_;

  protected:
    Texture_adapter mask_;

  public:
    static void init_rainbow() noexcept;

    static float3 spectrum_at_wavelength(float lambda, float value = 1.f) noexcept;

    static int32_t constexpr Num_bands = 16;

    using Spectrum = spectrum::Discrete_spectral_power_distribution<Num_bands>;

  private:
    static float3 rainbow_[Num_bands];
};

}  // namespace material

using Material_ptr = std::shared_ptr<material::Material>;
using Materials    = std::vector<Material_ptr>;

}  // namespace scene

#endif
