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
    Transformation const& transformation_at(uint64_t time, Transformation& transformation,
                                            Scene const& scene) const noexcept override final;

    bool sample(float3 const& p, float3 const& n, Transformation const& transformation,
                bool total_sphere, Sampler& sampler, uint32_t sampler_dimension,
                Worker const& worker, Sample_to& result) const noexcept override final;

    float3 evaluate(Sample_to const& sample, Filter filter, Worker const& worker) const
        noexcept override final;

    bool sample(Transformation const& transformation, Sampler& sampler, uint32_t sampler_dimension,
                AABB const& bounds, Worker const& worker, Sample_from& result) const
        noexcept override final;

    bool sample(Transformation const& transformation, Sampler& sampler, uint32_t sampler_dimension,
                Distribution_2D const& importance, AABB const& bounds, Worker const& worker,
                Sample_from& result) const noexcept override final;

    float3 evaluate(Sample_from const& sample, Filter filter, Worker const& worker) const
        noexcept override final;

    float pdf(Ray const& ray, Intersection const& intersection, bool total_sphere, Filter filter,
              Worker const& worker) const noexcept override final;

    float3 power(AABB const& scene_bb, Scene const& scene) const noexcept override final;

    void prepare_sampling(uint32_t light_id, uint64_t time, Scene const& scene,
                          thread::Pool& pool) noexcept override;

    bool equals(uint32_t prop, uint32_t part) const noexcept override final;
};

}  // namespace light
}  // namespace scene

#endif
