#ifndef SU_CORE_SCENE_LIGHT_NULL_LIGHT_HPP
#define SU_CORE_SCENE_LIGHT_NULL_LIGHT_HPP

#include "light.hpp"

namespace scene {

namespace shape {
class Node_stack;
}

namespace light {

class Null_light : public Light {
  public:
    virtual Transformation const& transformation_at(
        float time, Transformation& transformation) const override final;

    virtual bool sample(float3 const& p, float3 const& n, Transformation const& transformation,
                        bool total_sphere, sampler::Sampler& sampler, uint32_t sampler_dimension,
                        Worker const& worker, Sample_to& result) const override final;

    virtual bool sample(float3 const& p, Transformation const& transformation,
                        sampler::Sampler& sampler, uint32_t sampler_dimension, Worker const& worker,
                        Sample_to& result) const override final;

    virtual float3 evaluate(Sample_to const& sample, float time, Sampler_filter filter,
                            Worker const& worker) const override final;

    virtual bool sample(Transformation const& transformation, sampler::Sampler& sampler,
                        uint32_t sampler_dimension, math::AABB const& bounds, Worker const& worker,
                        Sample_from& result) const override final;

    virtual float3 evaluate(Sample_from const& sample, float time, Sampler_filter filter,
                            Worker const& worker) const override final;

    virtual float pdf(Ray const& ray, Intersection const& intersection, bool total_sphere,
                      Sampler_filter filter, Worker const& worker) const override final;

    virtual float3 power(math::AABB const& scene_bb) const override final;

    virtual void prepare_sampling(uint32_t light_id, thread::Pool& pool) override;

    virtual bool equals(Prop const* prop, uint32_t part) const override final;
};

}  // namespace light
}  // namespace scene

#endif
