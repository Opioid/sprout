#ifndef SU_EXTENSION_PROCEDURAL_AURORA_AURORA_HPP
#define SU_EXTENSION_PROCEDURAL_AURORA_AURORA_HPP

#include "core/scene/entity/entity.hpp"

namespace procedural::aurora {

class Aurora : public scene::entity::Entity {
  public:
    bool is_extension() const noexcept final override;

    void set_parameters(json::Value const& parameters) noexcept override final;

  private:
    void on_set_transformation() noexcept override final;
};

}  // namespace procedural::aurora

#endif
