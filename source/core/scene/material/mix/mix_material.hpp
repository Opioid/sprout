#pragma once

#include "scene/material/material.hpp"

namespace scene::material::mix {

class Material : public material::Material {
  public:
    Material(Sampler_settings const& sampler_settings, bool two_sided) noexcept;

    virtual material::Sample const& sample(float3 const& wo, Ray const& ray, Renderstate const& rs,
                                           Filter filter, sampler::Sampler& sampler,
                                           Worker const& worker, uint32_t depth) const
        noexcept override final;

    virtual float opacity(float2 uv, uint64_t time, Filter filter, Worker const& worker) const
        noexcept override final;

    virtual bool is_masked() const noexcept override final;

    virtual float ior() const noexcept override final;

    virtual size_t num_bytes() const noexcept override final;

    void set_materials(Material_ptr const& a, Material_ptr const& b) noexcept;

  private:
    Material_ptr material_a_;
    Material_ptr material_b_;
};

}  // namespace scene::material::mix
