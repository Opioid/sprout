#ifndef SU_EXTENSION_PROCEDURAL_SKY_HPP
#define SU_EXTENSION_PROCEDURAL_SKY_HPP

#include "base/math/matrix3x3.inl"
#include "core/scene/extension.hpp"
#include "sky_model.hpp"

namespace scene::prop {
class Prop;
}

namespace procedural::sky {

class Sky : public scene::Extension {
  public:
    using Scene = scene::Scene;

    Sky();

    ~Sky() override;

    void init(uint32_t sky, uint32_t sun, Scene& scene);

    void set_parameters(json::Value const& parameters, Scene& scene) final;

    Model& model();

    float3 sun_wi(float v) const;

    float sun_v(float3 const& wi) const;

    bool sky_changed_since_last_check();
    bool sun_changed_since_last_check();

    void update(Scene& scene) final override;

  private:
    void private_update(Scene& scene);

    //   void on_set_transformation()  final;

    Model model_;

    uint32_t sky_ = 0xFFFFFFFF;
    uint32_t sun_ = 0xFFFFFFFF;

    float3x3 sun_rotation_ = float3x3(1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, -1.f, 0.f);

    float3 ground_albedo_ = float3(0.2f, 0.2f, 0.2f);

    float turbidity_ = 2.f;

    bool implicit_rotation_ = true;

    bool sky_changed_;
    bool sun_changed_;
};

}  // namespace procedural::sky

#endif
