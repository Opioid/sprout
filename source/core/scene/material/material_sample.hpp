#ifndef SU_SCENE_MATERIAL_SAMPLE_HPP
#define SU_SCENE_MATERIAL_SAMPLE_HPP

#include "base/math/vector3.hpp"

namespace sampler {
class Sampler;
}

namespace scene {

struct Renderstate;
class Worker;

namespace material {

namespace bxdf {
struct Result;
struct Sample;
}  // namespace bxdf

struct Layer {
    Layer swapped() const;
    Layer swapped(bool same_side) const;

    void set_tangent_frame(float3 const& t, float3 const& b, float3 const& n);
    void set_tangent_frame(float3 const& n);

    float n_dot(float3 const& v) const;

    float abs_n_dot(float3 const& v) const;

    float clamp_n_dot(float3 const& v) const;

    float clamp_abs_n_dot(float3 const& v) const;

    float3 const& shading_normal() const;

    float3 tangent_to_world(float3 const& v) const;

    float3 world_to_tangent(float3 const& v) const;

    float3 t_;
    float3 b_;
    float3 n_;
};

class Sample {
  public:
    using Sampler = sampler::Sampler;

    virtual ~Sample();

    virtual float3 const& base_shading_normal() const = 0;

    virtual bxdf::Result evaluate_f(float3 const& wi) const = 0;

    virtual bxdf::Result evaluate_b(float3 const& wi) const = 0;

    virtual void sample(Sampler& sampler, bxdf::Sample& result) const = 0;

    virtual float3 radiance() const;

    virtual bool is_pure_emissive() const;

    virtual bool is_translucent() const;

    virtual bool ior_greater_one() const;

    float3 offset_p(float3 const& p, float3 const& wi) const;

    float3 const& wo() const;

    float clamp_geo_n_dot(float3 const& v) const;

    float3 const& geometric_normal() const;

    bool same_hemisphere(float3 const& v) const;

    void set_basis(float3 const& geo_n, float3 const& wo);

  protected:
    float3 geo_n_;
    float3 wo_;
};

struct IoR {
    IoR swapped() const;
    IoR swapped(bool same_side) const;

    float eta_t;
    float eta_i;
};

}  // namespace material
}  // namespace scene

#endif
