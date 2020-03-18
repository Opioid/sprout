#ifndef SU_CORE_SCENE_MATERIAL_MATERIAL_HPP
#define SU_CORE_SCENE_MATERIAL_MATERIAL_HPP

#include "base/flags/flags.hpp"
#include "base/math/vector3.hpp"
#include "base/spectrum/discrete.hpp"
#include "collision_coefficients.hpp"
#include "image/texture/texture_adapter.hpp"
#include "sampler_settings.hpp"

#include <string_view>

namespace math {
struct ray;
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

struct Ray;
struct Renderstate;
class Worker;
class Scene;

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
    using Filter          = Sampler_settings::Filter;
    using Shape           = shape::Shape;
    using Transformation  = entity::Composed_transformation;
    using Sampler         = sampler::Sampler;
    using Texture_adapter = image::texture::Adapter;

    static char const* identifier();

    Material(Sampler_settings const& sampler_settings, bool two_sided);

    virtual ~Material();

    void set_mask(Texture_adapter const& mask);

    void set_ior(float ior);

    virtual void commit(thread::Pool& threads, Scene const& scene);

    virtual void simulate(uint64_t start, uint64_t end, uint64_t frame_length,
                          thread::Pool& threads, Scene const& scene);

    virtual const Sample& sample(float3 const& wo, Ray const& ray, Renderstate const& rs,
                                 Filter filter, Sampler& sampler, Worker& worker) const = 0;

    virtual float3 evaluate_radiance(float3 const& wi, float2 uv, float area, Filter filter,
                                     Worker const& worker) const;

    virtual float3 evaluate_radiance(float3 const& wi, float3 const& uvw, float volume,
                                     Filter filter, Worker const& worker) const;

    virtual float3 average_radiance(float area_or_volume, Scene const& scene) const;

    struct Sample_2D {
        float2 uv;
        float  pdf;
    };
    virtual Sample_2D radiance_sample(float2 r2) const;

    virtual float emission_pdf(float2 uv, Filter filter, Worker const& worker) const;

    struct Sample_3D {
        float3 uvw;
        float  pdf;
    };
    virtual Sample_3D radiance_sample(float3 const& r3) const;

    virtual float emission_pdf(float3 const& uvw, Filter filter, Worker const& worker) const;

    virtual float opacity(float2 uv, uint64_t time, Filter filter, Worker const& worker) const;

    virtual float3 thin_absorption(float3 const& wi, float3 const& n, float2 uv, uint64_t time,
                                   Filter filter, Worker const& worker) const;

    float border(float3 const& wi, float3 const& n) const;

    virtual float3 absorption_coefficient(float2 uv, Filter filter, Worker const& worker) const;

    virtual CC collision_coefficients() const;

    virtual CC collision_coefficients(float2 uv, Filter filter, Worker const& worker) const;

    virtual CC collision_coefficients(float3 const& uvw, Filter filter, Worker const& worker) const;

    virtual CCE collision_coefficients_emission() const;

    virtual CCE collision_coefficients_emission(float3 const& uvw, Filter filter,
                                                Worker const& worker) const;

    virtual CM control_medium() const;

    virtual volumetric::Gridtree const* volume_tree() const;

    virtual float similarity_relation_scale(uint32_t depth) const;

    virtual void prepare_sampling(Shape const& shape, uint32_t part, uint64_t time,
                                  Transformation const& transformation, float extent,
                                  bool importance_sampling, thread::Pool& threads,
                                  Scene const& scene);

    uint32_t sampler_key() const;

    bool is_masked() const;

    bool is_two_sided() const;

    bool is_animated() const;

    bool is_caustic() const;

    bool has_tinted_shadow() const;

    bool has_emission_map() const;

    bool is_emissive(Scene const& scene) const;

    bool is_scattering_volume() const;

    bool is_textured_volume() const;

    bool is_heterogeneous_volume() const;

    float ior() const;

    virtual size_t num_bytes() const = 0;

  protected:
    uint32_t sampler_key_;

    enum class Property {
        None                 = 0,
        Two_sided            = 1 << 0,
        Animated             = 1 << 1,
        Caustic              = 1 << 2,
        Tinted_shadow        = 1 << 3,
        Emission_map         = 1 << 4,
        Scattering_volume    = 1 << 5,
        Textured_volume      = 1 << 6,
        Heterogeneous_volume = 1 << 7
    };

    flags::Flags<Property> properties_;

    Texture_adapter mask_;

    float ior_;

  public:
    static void init_rainbow();

    static float3 spectrum_at_wavelength(float lambda, float value = 1.f);

    static int32_t constexpr Num_bands = 36;

    using Spectrum = spectrum::Discrete_spectral_power_distribution<Num_bands>;

  private:
    static float3 rainbow_[Num_bands + 1];
};

}  // namespace material

}  // namespace scene

#endif
