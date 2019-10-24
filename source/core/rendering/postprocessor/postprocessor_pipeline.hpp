#ifndef SU_CORE_RENDERING_POSTPROCESSOR_PIPELINE_HPP
#define SU_CORE_RENDERING_POSTPROCESSOR_PIPELINE_HPP

#include "base/memory/array.hpp"
#include "image/typed_image.hpp"
#include "image/typed_image_fwd.hpp"

namespace scene::camera {
class Camera;
}

namespace thread {
class Pool;
}

namespace rendering {

namespace sensor {
class Sensor;
}

namespace postprocessor {

class Postprocessor;

class Pipeline {
  public:
    Pipeline() noexcept;

    ~Pipeline() noexcept;

    void clear() noexcept;

    void reserve(uint32_t num_pps) noexcept;

    void add(Postprocessor* pp) noexcept;

    void init(scene::camera::Camera const& camera, thread::Pool& pool) noexcept;

    bool has_alpha_transparency(bool alpha_in) const noexcept;

    void seed(sensor::Sensor const& sensor, image::Float4& target, thread::Pool& pool) noexcept;

    void apply(sensor::Sensor const& sensor, image::Float4& target, thread::Pool& pool) noexcept;

    void apply_accumulate(sensor::Sensor const& sensor, image::Float4& target,
                          thread::Pool& pool) noexcept;

  private:
    image::Float4 scratch_;

    memory::Array<Postprocessor*> postprocessors_;
};

}  // namespace postprocessor
}  // namespace rendering

#endif
