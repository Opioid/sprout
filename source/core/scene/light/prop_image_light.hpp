#ifndef SU_SCENE_LIGHT_PROP_IMAGE_LIGHT_HPP
#define SU_SCENE_LIGHT_PROP_IMAGE_LIGHT_HPP

#include "prop_light.hpp"

namespace scene::light {

class Prop_image_light final : public Prop_light {
  public:
    bool sample(float3 const& p, float3 const& n, Transformation const& transformation,
                bool total_sphere, Sampler& sampler, uint32_t sampler_dimension,
                Worker const& worker, Sample_to& result) const noexcept override final;

    bool sample(Transformation const& transformation, Sampler& sampler, uint32_t sampler_dimension,
                AABB const& bounds, Worker const& worker, Sample_from& result) const
        noexcept override final;

    bool sample(Transformation const& transformation, Sampler& sampler, uint32_t sampler_dimension,
                Distribution_2D const& importance, AABB const& bounds, Worker const& worker,
                Sample_from& result) const noexcept override final;

    float pdf(Ray const& ray, Intersection const& intersection, bool total_sphere, Filter filter,
              Worker const& worker) const noexcept override final;

    void prepare_sampling(uint32_t light_id, uint64_t time, Scene& scene,
                          thread::Pool& pool) noexcept override final;
};

}  // namespace scene::light

#endif
