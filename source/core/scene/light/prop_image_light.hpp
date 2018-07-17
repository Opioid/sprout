#ifndef SU_SCENE_LIGHT_PROP_IMAGE_LIGHT_HPP
#define SU_SCENE_LIGHT_PROP_IMAGE_LIGHT_HPP

#include "prop_light.hpp"

namespace scene::light {

class Prop_image_light : public Prop_light {
  public:
    virtual bool sample(f_float3 p, f_float3 n, Transformation const& transformation,
                        bool total_sphere, sampler::Sampler& sampler, uint32_t sampler_dimension,
                        Worker const& worker, Sample_to& result) const override final;

    virtual bool sample(f_float3 p, Transformation const& transformation, sampler::Sampler& sampler,
                        uint32_t sampler_dimension, Worker const& worker,
                        Sample_to& result) const override final;

    virtual bool sample(Transformation const& transformation, sampler::Sampler& sampler,
                        uint32_t sampler_dimension, math::AABB const& bounds, Worker const& worker,
                        Sample_from& result) const override final;

    virtual float pdf(Ray const& ray, Intersection const& intersection, bool total_sphere,
                      Sampler_filter filter, Worker const& worker) const override final;

    virtual void prepare_sampling(uint32_t light_id, thread::Pool& pool) override final;
};

}  // namespace scene::light

#endif
