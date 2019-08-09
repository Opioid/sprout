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
    Layer swapped() const noexcept;
    Layer swapped(bool same_side) const noexcept;

    void set_tangent_frame(float3 const& t, float3 const& b, float3 const& n) noexcept;
    void set_tangent_frame(float3 const& n) noexcept;

    float n_dot(float3 const& v) const noexcept;
    float abs_n_dot(float3 const& v) const noexcept;

    float clamp_n_dot(float3 const& v) const noexcept;
    float clamp_abs_n_dot(float3 const& v) const noexcept;

    float3 const& shading_normal() const noexcept;

    float3 tangent_to_world(float3 const& v) const noexcept;

    float3 world_to_tangent(float3 const& v) const noexcept;

    float3 t_;
    float3 b_;
    float3 n_;
};

class Sample {
  public:
    using Sampler = sampler::Sampler;

    virtual ~Sample();

    virtual Layer const& base_layer() const noexcept = 0;

    virtual bxdf::Result evaluate_f(float3 const& wi, bool include_back) const noexcept = 0;

    virtual bxdf::Result evaluate_b(float3 const& wi, bool include_back) const noexcept = 0;

    virtual void sample(Sampler& sampler, bxdf::Sample& result) const noexcept = 0;

    virtual float3 radiance() const noexcept;

    virtual bool is_pure_emissive() const noexcept;

    virtual bool is_translucent() const noexcept;

    virtual bool ior_greater_one() const noexcept;

    virtual bool do_evaluate_back(bool previously, bool same_side) const noexcept;

    float3 offset_p(float3 const& p) const noexcept;
    float3 offset_p(float3 const& p, float3 const& wi) const noexcept;

    float3 const& wo() const noexcept;

    float clamp_geo_n_dot(float3 const& v) const noexcept;

    float3 const& geometric_normal() const noexcept;

    bool same_hemisphere(float3 const& v) const noexcept;

    void set_basis(float3 const& geo_n, float3 const& wo) noexcept;

  protected:
    float3 geo_n_;
    float3 wo_;
};

struct IoR {
    IoR swapped() const noexcept;
    IoR swapped(bool same_side) const noexcept;

    float eta_t;
    float eta_i;
};

}  // namespace material
}  // namespace scene

#endif
