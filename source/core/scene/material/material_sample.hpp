#ifndef SU_SCENE_MATERIAL_SAMPLE_HPP
#define SU_SCENE_MATERIAL_SAMPLE_HPP

#include "base/math/vector3.hpp"

namespace sampler {
class Sampler;
}

namespace scene {

class Worker;

namespace material {

namespace bxdf {
struct Result;
struct Sample;
}  // namespace bxdf

class Sample {
  public:
    virtual ~Sample();

    struct Layer {
        void set_tangent_frame(float3 const& t, float3 const& b, float3 const& n) noexcept;
        void set_tangent_frame(float3 const& n) noexcept;

        float abs_n_dot(float3 const& v) const noexcept;

        float clamp_n_dot(float3 const& v) const noexcept;
        float clamp_abs_n_dot(float3 const& v) const noexcept;
        float clamp_reverse_n_dot(float3 const& v) const noexcept;

        float3 const& shading_normal() const noexcept;

        float3 tangent_to_world(float3 const& v) const noexcept;

        float3 world_to_tangent(float3 const& v) const noexcept;

        float3 t_;
        float3 b_;
        float3 n_;
    };

    virtual Layer const& base_layer() const noexcept = 0;

    virtual bxdf::Result evaluate(float3 const& wi) const noexcept = 0;

    virtual void sample(sampler::Sampler& sampler, bxdf::Sample& result) const noexcept = 0;

    virtual float3 radiance() const noexcept;

    virtual bool is_pure_emissive() const noexcept;

    virtual bool is_translucent() const noexcept;

    virtual bool ior_greater_one() const noexcept;

    float3 const& wo() const noexcept;

    float clamp_geo_n_dot(float3 const& v) const noexcept;
    float clamp_reverse_geo_n_dot(float3 const& v) const noexcept;

    float3 const& geometric_normal() const noexcept;

    bool same_hemisphere(float3 const& v) const noexcept;

    void set_basis(float3 const& geo_n, float3 const& wo) noexcept;

  protected:
    float3 geo_n_;
    float3 wo_;
};

}  // namespace material
}  // namespace scene

#endif
