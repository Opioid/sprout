#pragma once

#include "core/scene/material/material.hpp"

namespace procedural {
namespace sky {

class Model;

class Material : public scene::material::Material {
  public:
    Material(Model& model);

  protected:
    Model& model_;
};

}  // namespace sky
}  // namespace procedural
