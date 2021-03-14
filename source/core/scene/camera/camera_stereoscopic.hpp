#ifndef SU_CORE_SCENE_CAMERA_STEREOSCOPIC_HPP
#define SU_CORE_SCENE_CAMERA_STEREOSCOPIC_HPP

#include "camera.hpp"
#include "base/math/vector3.hpp"

namespace scene::camera {

class Stereoscopic : public Camera {
  public:
    Stereoscopic();

    virtual ~Stereoscopic();

    void set_interpupillary_distance(float ipd);

  protected:
    float3 eye_offsets_[2];
};

}  // namespace scene::camera

#endif
