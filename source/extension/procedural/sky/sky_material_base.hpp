#ifndef SU_EXTENSION_PROCEDURAL_SKY_BASE_MATERIAL_HPP
#define SU_EXTENSION_PROCEDURAL_SKY_BASE_MATERIAL_HPP

#include "core/scene/material/material.hpp"

namespace procedural::sky {

class Sky;

class Material : public scene::material::Material {
  public:
    Material(Sky& sky);

    float ior() const final;

  protected:
    Sky& sky_;
};

}  // namespace procedural::sky

#endif
