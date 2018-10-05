#ifndef SU_CORE_RENDERING_POSTPROCESSOR_BACKPLATE_HPP
#define SU_CORE_RENDERING_POSTPROCESSOR_BACKPLATE_HPP

#include "image/texture/texture_types.hpp"
#include "postprocessor.hpp"

namespace rendering::postprocessor {

class Backplate : public Postprocessor {
  public:
    Backplate(Texture_ptr const& backplate);

    virtual void init(scene::camera::Camera const& camera, thread::Pool& pool) override final;

    virtual bool alpha_out(bool alpha_in) const override final;

    virtual size_t num_bytes() const override final;

  private:
    virtual void apply(uint32_t id, uint32_t pass, int32_t begin, int32_t end,
                       const image::Float4& source, image::Float4& destination) override final;

    Texture_ptr backplate_;
};

}  // namespace rendering::postprocessor

#endif
