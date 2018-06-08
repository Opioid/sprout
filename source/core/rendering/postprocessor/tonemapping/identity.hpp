#pragma once

#include "tonemapper.hpp"

namespace rendering {
namespace postprocessor {
namespace tonemapping {

class Identity : public Tonemapper {
  private:
    virtual void apply(uint32_t id, uint32_t pass, int32_t begin, int32_t end,
                       const image::Float4& source, image::Float4& destination) override final;
};

}  // namespace tonemapping
}  // namespace postprocessor
}  // namespace rendering
