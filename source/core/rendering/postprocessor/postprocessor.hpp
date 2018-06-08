#ifndef SU_CORE_RENDERING_POSTPROCESSOR_POSTPROCESSOR_HPP
#define SU_CORE_RENDERING_POSTPROCESSOR_POSTPROCESSOR_HPP

#include "image/typed_image_fwd.hpp"
#include "scene/camera/camera.hpp"

namespace thread {
class Pool;
}

namespace rendering::postprocessor {

class Postprocessor {
  public:
    Postprocessor(uint32_t num_passes = 1);
    virtual ~Postprocessor();

    virtual void init(const scene::camera::Camera& camera, thread::Pool& pool) = 0;

    virtual size_t num_bytes() const = 0;

    virtual bool alpha_out(bool alpha_in) const;

    void apply(const image::Float4& source, image::Float4& destination, thread::Pool& pool);

  private:
    virtual void pre_apply(const image::Float4& source, image::Float4& destination,
                           thread::Pool& pool);

    virtual void apply(uint32_t id, uint32_t pass, int32_t begin, int32_t end,
                       const image::Float4& source, image::Float4& destination) = 0;

    uint32_t num_passes_;
};

}  // namespace rendering::postprocessor

#endif
