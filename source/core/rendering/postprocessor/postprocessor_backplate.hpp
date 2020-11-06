#ifndef SU_CORE_RENDERING_POSTPROCESSOR_BACKPLATE_HPP
#define SU_CORE_RENDERING_POSTPROCESSOR_BACKPLATE_HPP

#include "image/texture/texture_types.hpp"
#include "postprocessor.hpp"

namespace rendering::postprocessor {

class Backplate : public Postprocessor {
  public:
    Backplate(image::texture::Texture const* backplate);

    void init(scene::camera::Camera const& camera, Threads& threads) final;

    bool alpha_out(bool alpha_in) const final;

  private:
    void apply(uint32_t id, uint32_t pass, int32_t begin, int32_t end, image::Float4 const& source,
               image::Float4& destination) final;

    image::texture::Texture const* backplate_;
};

}  // namespace rendering::postprocessor

#endif
