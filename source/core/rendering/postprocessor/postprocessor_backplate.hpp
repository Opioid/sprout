#ifndef SU_CORE_RENDERING_POSTPROCESSOR_BACKPLATE_HPP
#define SU_CORE_RENDERING_POSTPROCESSOR_BACKPLATE_HPP

#include "image/texture/texture_types.hpp"
#include "postprocessor.hpp"

namespace rendering::postprocessor {

class Backplate : public Postprocessor {
  public:
    Backplate(image::texture::Texture* backplate);

    void init(scene::camera::Camera const& camera, thread::Pool& pool) override final;

    bool alpha_out(bool alpha_in) const override final;

    size_t num_bytes() const override final;

  private:
    void apply(uint32_t id, uint32_t pass, int32_t begin, int32_t end, image::Float4 const& source,
               image::Float4& destination) override final;

    image::texture::Texture* backplate_;
};

}  // namespace rendering::postprocessor

#endif
