#ifndef SU_EXTENSION_PROCEDURAL_AURORA_AURORA_HPP
#define SU_EXTENSION_PROCEDURAL_AURORA_AURORA_HPP

#include "core/scene/entity/entity.hpp"

namespace procedural::aurora {

class Aurora : public scene::entity::Entity {
  public:
    virtual void set_parameters(json::Value const& parameters) override final;

  private:
    virtual void on_set_transformation() override final;
};

}  // namespace procedural::aurora

#endif
