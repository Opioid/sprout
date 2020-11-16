#ifndef SU_CORE_SCENE_MATERIAL_FRESNEL_HPP
#define SU_CORE_SCENE_MATERIAL_FRESNEL_HPP

#include "base/math/vector3.hpp"

namespace scene::material::fresnel {

class Schlick1 {
  public:
    Schlick1(float f0);

    float operator()(float wo_dot_h) const;

  private:
    float const f0_;
};

class Schlick {
  public:
    Schlick(float f0);
    Schlick(float3_p f0);

    float3 operator()(float wo_dot_h) const;

  private:
    float3 const f0_;
};

class Lazanyi_schlick {
  public:
    Lazanyi_schlick(float f0, float a);
    Lazanyi_schlick(float3_p f0, float3_p a);

    float3 operator()(float wo_dot_h) const;

  private:
    float3 const f0_;
    float3 const a_;
};

class Thinfilm {
  public:
    Thinfilm(float external_ior, float thinfilm_ior, float internal_ior, float thickness);

    float3 operator()(float wo_dot_h) const;

  private:
    float const external_ior_;
    float const thinfilm_ior_;
    float const internal_ior_;
    float const thickness_;
};

class Dielectric {
  public:
    Dielectric(float eta_i, float eta_t);

    float3 operator()(float wo_dot_h) const;

  private:
    float const eta_i_;
    float const eta_t_;
};

class Conductor {
  public:
    Conductor(float3_p eta, float3_p k);

    float3 operator()(float wo_dot_h) const;

  private:
    float3 const eta_;
    float3 const k_;
};

class Constant {
  public:
    Constant(float f);
    Constant(float3_p f);

    float3 operator()(float wo_dot_h) const;

  private:
    float3 const f_;
};

}  // namespace scene::material::fresnel

#endif
