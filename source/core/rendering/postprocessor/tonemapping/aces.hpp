#ifndef SU_CORE_RENDERING_POSTPROCESSOR_TONEMAPPING_ACES_HPP
#define SU_CORE_RENDERING_POSTPROCESSOR_TONEMAPPING_ACES_HPP

#include "tonemapper.hpp"

namespace rendering::postprocessor::tonemapping {

class Aces : public Tonemapper {
  public:
    Aces(bool auto_expose, float exposure);

  private:
    void apply(uint32_t id, uint32_t pass, int32_t begin, int32_t end, image::Float4 const& source,
               image::Float4& destination) final;
};

}  // namespace rendering::postprocessor::tonemapping

#endif
