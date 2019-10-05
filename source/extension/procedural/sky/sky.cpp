#include "sky.hpp"
#include "base/json/json.hpp"
#include "base/math/matrix3x3.inl"
#include "base/math/matrix4x4.inl"
#include "base/math/quaternion.inl"
#include "base/math/vector3.inl"
#include "core/scene/prop/prop.hpp"
#include "core/scene/scene.inl"
#include "core/scene/scene_constants.hpp"

namespace procedural::sky {

Sky::Sky() noexcept = default;

Sky::~Sky() noexcept {}

void Sky::init(uint32_t sky, uint32_t sun, Scene& scene) noexcept {
    sky_ = sky;
    sun_ = sun;

    const math::Transformation transformation{
        float3(0.f), float3(1.f),
        math::quaternion::create_rotation_x(math::degrees_to_radians(90.f))};

    scene.prop_set_transformation(sky, transformation);
}

void Sky::set_parameters(json::Value const& parameters, Scene& scene) noexcept {
    for (auto& n : parameters.GetObject()) {
        if ("sun" == n.name) {
            float3 const angles = json::read_float3(n.value, "rotation", float3(0.f));

            sun_rotation_      = json::create_rotation_matrix(angles);
            implicit_rotation_ = false;
        } else if ("ground_albedo" == n.name) {
            ground_albedo_ = json::read_float3(n.value);
        } else if ("turbidity" == n.name) {
            turbidity_ = json::read_float(n.value);
        }
    }

    private_update(scene);
}

Model& Sky::model() noexcept {
    return model_;
}

float3 Sky::sun_wi(float v) const noexcept {
    float const y = (2.f * v) - 1.f;

    float3 const ls = float3(0.f, y * Model::radius(), 0.f);

    float3 const ws = transform_vector(sun_rotation_, ls);

    return normalize(ws - sun_rotation_.r[2]);
}

float Sky::sun_v(float3 const& wi) const noexcept {
    float3 const k = wi - sun_rotation_.r[2];

    float const c = dot(sun_rotation_.r[1], k) / Model::radius();

    return std::max((c + 1.f) * 0.5f, 0.f);
}

bool Sky::sky_changed_since_last_check() noexcept {
    bool const sky_changed = sky_changed_;

    sky_changed_ = false;

    return sky_changed;
}

bool Sky::sun_changed_since_last_check() noexcept {
    bool const sun_changed = sun_changed_;

    sun_changed_ = false;

    return sun_changed;
}

void Sky::update(Scene& scene) noexcept {
    scene::prop::Prop* const e = scene.prop(prop_);

    scene::prop::Prop* const sky = scene.prop(sky_);
    scene::prop::Prop* const sun = scene.prop(sun_);

    sky->set_visibility(e->visible_in_camera(), e->visible_in_reflection(), e->visible_in_shadow());
    sun->set_visibility(e->visible_in_camera(), e->visible_in_reflection(), e->visible_in_shadow());

    if (implicit_rotation_) {
        sun_rotation_ = math::quaternion::create_matrix3x3(
            scene.prop_local_frame_0(prop_).rotation);

        private_update(scene);
    }
}

void Sky::private_update(Scene& scene) noexcept {
    model_.set_sun_direction(sun_rotation_.r[2]);
    model_.set_ground_albedo(ground_albedo_);
    model_.set_turbidity(turbidity_);

    math::Transformation const transformation{float3(0.f), float3(Model::radius()),
                                              math::quaternion::create(sun_rotation_)};

    scene.prop_set_transformation(sun_, transformation);

    model_.compile();

    sky_changed_ = true;
    sun_changed_ = true;
}

}  // namespace procedural::sky
