#pragma once

#include "scene/material/material.hpp"

namespace scene::material::mix {

class Material : public material::Material {
  public:
    Material(Sampler_settings const& sampler_settings, bool two_sided);

    material::Sample const& sample(float3 const& wo, Ray const& ray, Renderstate const& rs,
                                   Filter filter, Sampler& sampler,
                                   Worker const& worker) const final;

    float opacity(float2 uv, uint64_t time, Filter filter, Worker const& worker) const final;

    bool is_masked() const final;

    float ior() const final;

    size_t num_bytes() const final;

    void set_materials(material::Material const* a, material::Material const* const b);

  private:
    material::Material const* material_a_;
    material::Material const* material_b_;
};

}  // namespace scene::material::mix
