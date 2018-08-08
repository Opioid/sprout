#ifndef SU_EXTENSION_PROCEDURAL_SKY_HPP
#define SU_EXTENSION_PROCEDURAL_SKY_HPP

#include "base/math/matrix3x3.inl"
#include "core/scene/entity/entity.hpp"
#include "sky_model.hpp"

namespace scene::prop {
class Prop;
}

namespace procedural::sky {

class Sky : public scene::entity::Entity {
  public:
    Sky() noexcept = default;

    ~Sky() noexcept override;

    void set_parameters(json::Value const& parameters) noexcept override final;

    void init(scene::prop::Prop* sky, scene::prop::Prop* sun) noexcept;

    Model& model() noexcept;

    float3 sun_wi(float v) const noexcept;

    float sun_v(float3 const& wi) const noexcept;

    bool sky_changed_since_last_check() noexcept;
    bool sun_changed_since_last_check() noexcept;

  private:
    void update() noexcept;

    void on_set_transformation() noexcept override final;

    Model model_;

    scene::prop::Prop* sky_;
    scene::prop::Prop* sun_;

    float3x3 sun_rotation_ = float3x3(1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, -1.f, 0.f);

    float3 ground_albedo_ = float3(0.2f, 0.2f, 0.2f);

    float turbidity_ = 2.f;

    bool implicit_rotation_ = true;

    bool sky_changed_;
    bool sun_changed_;
};

}  // namespace procedural::sky

#endif
