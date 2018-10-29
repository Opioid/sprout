#ifndef SU_CORE_SCENE_ENTITY_DUMMY_HPP
#define SU_CORE_SCENE_ENTITY_DUMMY_HPP

#include "entity.hpp"

namespace scene::entity {

class Dummy : public Entity {
  public:
    void set_parameters(json::Value const& parameters) noexcept override final;

  private:
    void on_set_transformation() noexcept override final;
};

}  // namespace scene::entity

#endif
