#ifndef SU_CORE_RENDERING_POSTPROCESSOR_POSTPROCESSOR_HPP
#define SU_CORE_RENDERING_POSTPROCESSOR_POSTPROCESSOR_HPP

#include "image/typed_image_fwd.hpp"

namespace scene::camera {
class Camera;
}

namespace thread {
class Pool;
}

using Threads = thread::Pool;

namespace rendering::postprocessor {

class Postprocessor {
  public:
    using Camera = scene::camera::Camera;

    Postprocessor(uint32_t num_passes = 1);

    virtual ~Postprocessor();

    virtual void init(Camera const& camera, Threads& threads) = 0;

    virtual bool alpha_out(bool alpha_in) const;

    void apply(image::Float4 const& source, image::Float4& destination, Threads& threads);

  private:
    virtual void pre_apply(image::Float4 const& source, image::Float4& destination,
                           Threads& threads);

    virtual void apply(uint32_t id, uint32_t pass, int32_t begin, int32_t end,
                       image::Float4 const& source, image::Float4& destination) = 0;

    virtual void post_pass(uint32_t pass);

    uint32_t num_passes_;
};

}  // namespace rendering::postprocessor

#endif
