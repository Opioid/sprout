#ifndef SU_EXTENSION_PROCEDURAL_FLAME_FLAME_HPP
#define SU_EXTENSION_PROCEDURAL_FLAME_FLAME_HPP

#include "core/scene/entity/entity.hpp"

namespace procedural::flame {

class Flame : public scene::entity::Entity {
  public:
    void set_parameters(json::Value const& parameters) noexcept override final;

  private:
    void on_set_transformation() noexcept override final;
};

}  // namespace procedural::flame

#endif
