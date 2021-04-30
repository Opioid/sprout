#ifndef SU_CORE_RENDERING_POSTPROCESSOR_PIPELINE_HPP
#define SU_CORE_RENDERING_POSTPROCESSOR_PIPELINE_HPP

#include "base/memory/array.hpp"
#include "image/typed_image.hpp"
#include "image/typed_image_fwd.hpp"

namespace scene {
class Scene;
namespace camera {
class Camera;
}
}  // namespace scene

namespace thread {
class Pool;
}

using Threads = thread::Pool;

namespace rendering {

namespace sensor {
class Sensor;
}

namespace postprocessor {

class Postprocessor;

class Pipeline {
  public:
    using Scene = scene::Scene;

    Pipeline();

    ~Pipeline();

    bool empty() const;

    void clear();

    void reserve(uint32_t num_pps);

    void add(Postprocessor* pp);

    void init(scene::camera::Camera const& camera, Threads& threads);

    bool has_alpha_transparency(bool alpha_in) const;

    void seed(sensor::Sensor const& sensor, image::Float4& target, Threads& threads);

    void apply(image::Float4& target, Scene const& scene, Threads& threads);

    void apply(sensor::Sensor const& sensor, image::Float4& target, Scene const& scene,
               Threads& threads);

    void apply_accumulate(sensor::Sensor const& sensor, image::Float4& target, Scene const& scene,
                          Threads& threads);

  private:
    image::Float4 scratch_;

    memory::Array<Postprocessor*> postprocessors_;
};

}  // namespace postprocessor
}  // namespace rendering

#endif
