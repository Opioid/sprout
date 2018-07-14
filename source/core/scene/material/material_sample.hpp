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
        void set_tangent_frame(f_float3 t, f_float3 b, f_float3 n);
        void set_tangent_frame(f_float3 n);

        float abs_n_dot(f_float3 v) const;

        float clamp_n_dot(f_float3 v) const;
        float clamp_abs_n_dot(f_float3 v) const;
        float clamp_reverse_n_dot(f_float3 v) const;

        f_float3 shading_normal() const;

        float3 tangent_to_world(f_float3 v) const;

        float3 world_to_tangent(f_float3 v) const;

        float3 t_;
        float3 b_;
        float3 n_;
    };

    virtual Layer const& base_layer() const = 0;

    virtual bxdf::Result evaluate(f_float3 wi) const = 0;

    virtual void sample(sampler::Sampler& sampler, bxdf::Sample& result) const = 0;

    virtual float3 radiance() const;

    virtual bool is_pure_emissive() const;

    virtual bool is_translucent() const;

    virtual bool ior_greater_one() const;

    f_float3 wo() const;

    float clamp_geo_n_dot(f_float3 v) const;
    float clamp_reverse_geo_n_dot(f_float3 v) const;

    f_float3 geometric_normal() const;

    bool same_hemisphere(f_float3 v) const;

    void set_basis(f_float3 geo_n, f_float3 wo);

  protected:
    float3 geo_n_;
    float3 wo_;
};

}  // namespace material
}  // namespace scene

#endif
