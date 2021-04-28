#ifndef SU_CORE_SCENE_MATERIAL_MATERIAL_HPP
#define SU_CORE_SCENE_MATERIAL_MATERIAL_HPP

#include "base/flags/flags.hpp"
#include "base/math/vector3.hpp"
#include "base/spectrum/discrete.hpp"
#include "collision_coefficients.hpp"
#include "image/texture/texture_adapter.hpp"
#include "image/texture/texture.hpp"
#include "sampler_settings.hpp"

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

using Threads = thread::Pool;

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
    using Turbotexture = image::texture::Turbotexture;

    static char const* identifier();

    Material(Sampler_settings sampler_settings, bool two_sided);

    virtual ~Material();

    void set_mask(Texture_adapter const& mask);

    void set_color_map(Texture_adapter const& color_map);

    void set_emission(float3_p emission);

    void set_ior(float ior);

    void set_volumetric(float3_p attenuation_color, float3_p subsurface_color, float distance,
                        float anisotropy);

    virtual void commit(Threads& threads, Scene const& scene);

    virtual void simulate(uint64_t start, uint64_t end, uint64_t frame_length, Threads& threads,
                          Scene const& scene);

    virtual Sample const& sample(float3_p wo, Ray const& ray, Renderstate const& rs, Filter filter,
                                 Sampler& sampler, Worker& worker) const = 0;

    virtual float3 average_radiance(float extent) const;

    virtual float3 evaluate_radiance(float3_p wi, float3_p uvw, float extent, Filter filter,
                                     Worker const& worker) const;

    struct Radiance_sample {
        Radiance_sample(float2 uv, float pdf);

        Radiance_sample(float3_p uvw, float pdf);

        float pdf() const;

        float3 uvw;
    };
    virtual Radiance_sample radiance_sample(float3_p r3) const;

    virtual float emission_pdf(float3_p uvw, Filter filter, Worker const& worker) const;

    float opacity(float2 uv, Filter filter, Worker const& worker) const;

    virtual float3 thin_absorption(float3_p wi, float3_p n, float2 uv, Filter filter,
                                   Worker const& worker) const;

    float border(float3_p wi, float3_p n) const;

    CC collision_coefficients() const;

    CC collision_coefficients(float2 uv, Filter filter, Worker const& worker) const;

    virtual CC collision_coefficients(float3_p uvw, Filter filter, Worker const& worker) const;

    CCE collision_coefficients_emission() const;

    virtual CCE collision_coefficients_emission(float3_p uvw, Filter filter,
                                                Worker const& worker) const;

    virtual volumetric::Gridtree const* volume_tree() const;

    struct Boxi {
        int3 min;
        int3 max;
    };
    virtual Boxi volume_texture_space_bounds(Scene const& scene) const;

    float phase(float3_p wo, float3_p wi) const;

    float4 sample_phase(float3_p wo, float2 r2) const;

    float similarity_relation_scale(uint32_t depth) const;

    virtual void prepare_sampling(Shape const& shape, uint32_t part, Transformation const& trafo,
                                  float extent, bool importance_sampling, Threads& threads,
                                  Scene const& scene);

    uint32_t sampler_key() const;

    bool is_masked() const;
    bool is_two_sided() const;
    bool is_animated() const;
    bool is_caustic() const;
    bool has_tinted_shadow() const;
    bool has_emission_map() const;
    bool is_emissive() const;
    bool is_pure_emissive() const;
    bool is_scattering_volume() const;
    bool is_textured_volume() const;
    bool is_heterogeneous_volume() const;

    float ior() const;

  protected:
    float van_de_hulst_anisotropy(uint32_t depth) const;

    uint32_t const sampler_key_;

    enum class Property {
        None                 = 0,
        Two_sided            = 1 << 0,
        Animated             = 1 << 1,
        Caustic              = 1 << 2,
        Tinted_shadow        = 1 << 3,
        Emission_map         = 1 << 4,
        Pure_emisive         = 1 << 5,
        Scattering_volume    = 1 << 6,
        Textured_volume      = 1 << 7,
        Heterogeneous_volume = 1 << 8
    };

    flags::Flags<Property> properties_;

    Texture_adapter mask_;
    Texture_adapter color_map_;

    Turbotexture color_maply_;

    CC cc_;

    float3 emission_;

    float ior_;
    float attenuation_distance_;
    float volumetric_anisotropy_;

    int32_t element_;

  public:
    static void init_rainbow();

    static float3 spectrum_at_wavelength(float lambda, float value = 1.f);

    static void set_similarity_relation_range(uint32_t low, uint32_t high);

    static int32_t constexpr Num_bands = 36;

    using Spectrum = spectrum::Discrete_spectral_power_distribution<Num_bands>;

  private:
    static float3 rainbow_[Num_bands + 1];

    static uint32_t SR_low;
    static uint32_t SR_high;
    static float    SR_inv_range;
};

}  // namespace material

}  // namespace scene

#endif
