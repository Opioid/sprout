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
    Pipeline();

    ~Pipeline();

    void reserve(size_t num_pps);

    void add(Postprocessor* pp);

    void init(scene::camera::Camera const& camera, thread::Pool& pool);

    bool has_alpha_transparency(bool alpha_in) const;

    void apply(sensor::Sensor const& sensor, image::Float4& target, thread::Pool& pool);

    size_t num_bytes() const;

  private:
    image::Float4 scratch_;

    memory::Array<Postprocessor*> postprocessors_;
};

}  // namespace postprocessor
}  // namespace rendering

#endif
