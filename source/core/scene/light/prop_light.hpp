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
    void init(Prop* prop, uint32_t part) noexcept;

    Transformation const& transformation_at(uint64_t time, Transformation& transformation) const
        noexcept override final;

    bool sample(float3 const& p, float3 const& n, Transformation const& transformation,
                bool total_sphere, sampler::Sampler& sampler, uint32_t sampler_dimension,
                Worker const& worker, Sample_to& result) const noexcept override;

    bool sample(float3 const& p, Transformation const& transformation, sampler::Sampler& sampler,
                uint32_t sampler_dimension, Worker const& worker, Sample_to& result) const
        noexcept override;

    float3 evaluate(Sample_to const& sample, uint64_t time, Sampler_filter filter,
                    Worker const& worker) const noexcept override final;

    bool sample(Transformation const& transformation, sampler::Sampler& sampler,
                uint32_t sampler_dimension, math::AABB const& bounds, Worker const& worker,
                Sample_from& result) const noexcept override;

    float3 evaluate(Sample_from const& sample, uint64_t time, Sampler_filter filter,
                    Worker const& worker) const noexcept override final;

    float pdf(Ray const& ray, Intersection const& intersection, bool total_sphere,
              Sampler_filter filter, Worker const& worker) const noexcept override;

    float3 power(math::AABB const& scene_bb) const noexcept override final;

    void prepare_sampling(uint32_t light_id, uint64_t time, thread::Pool& pool) noexcept override;

    bool equals(Prop const* prop, uint32_t part) const noexcept override final;

  protected:
    Prop* prop_;

    uint32_t part_;
};

}  // namespace light
}  // namespace scene

#endif
