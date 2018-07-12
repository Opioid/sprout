#ifndef SU_SCENE_LIGHT_PROP_LIGHT_HPP
#define SU_SCENE_LIGHT_PROP_LIGHT_HPP

#include "light.hpp"

namespace scene {

namespace shape {
class Node_stack;
}

namespace light {

class Prop_light : public Light {
  public:
    void init(Prop* prop, uint32_t part);

    virtual Transformation const& transformation_at(
        float time, Transformation& transformation) const override final;

    virtual bool sample(f_float3 p, f_float3 n, Transformation const& transformation,
                        bool total_sphere, sampler::Sampler& sampler, uint32_t sampler_dimension,
                        Worker const& worker, Sample_to& result) const override;

    virtual bool sample(f_float3 p, Transformation const& transformation, sampler::Sampler& sampler,
                        uint32_t sampler_dimension, Worker const& worker,
                        Sample_to& result) const override;

    virtual bool sample(float time, Transformation const& transformation, sampler::Sampler& sampler,
                        uint32_t sampler_dimension, Sampler_filter filter, Worker const& worker,
                        Sample_from& result) const override;

    virtual float3 evaluate_radiance(Sample_to const& sample, float time, Sampler_filter filter,
                                     Worker const& worker) const override final;

    virtual float pdf(Ray const& ray, Intersection const& intersection, bool total_sphere,
                      Sampler_filter filter, Worker const& worker) const override;

    virtual float3 power(math::AABB const& scene_bb) const override final;

    virtual void prepare_sampling(uint32_t light_id, thread::Pool& pool) override;

    virtual bool equals(Prop const* prop, uint32_t part) const override final;

  protected:
    Prop* prop_;

    uint32_t part_;
};

}  // namespace light
}  // namespace scene

#endif
