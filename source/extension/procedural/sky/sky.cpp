#include "sky.hpp"
#include "base/json/json.hpp"
#include "base/math/matrix3x3.inl"
#include "base/math/matrix4x4.inl"
#include "base/math/quaternion.inl"
#include "base/math/vector3.inl"
#include "core/scene/prop/prop.hpp"

namespace procedural::sky {

Sky::~Sky() {}

void Sky::init(scene::prop::Prop* sky, scene::prop::Prop* sun) {
    sky_ = sky;
    sun_ = sun;

    attach(sky_);
    attach(sun_);

    update();

    const math::Transformation transformation{
        float3::identity(), float3(1.f),
        math::quaternion::create_rotation_x(math::degrees_to_radians(90.f))};

    sky->set_transformation(transformation);
}

void Sky::set_parameters(json::Value const& parameters) {
    for (auto& n : parameters.GetObject()) {
        if ("sun" == n.name) {
            float3 const angles = json::read_float3(n.value, "rotation", float3::identity());
            sun_rotation_       = json::create_rotation_matrix(angles);

            // Supposedly the sun direction in the disney cloud scene
//            sun_rotation_.r[2] = float3(-0.5826f, -0.7660f, -0.2717f);
//            math::orthonormal_basis(sun_rotation_.r[2], sun_rotation_.r[0], sun_rotation_.r[1]);

            implicit_rotation_ = false;
        } else if ("ground_albedo" == n.name) {
            ground_albedo_ = json::read_float3(n.value);
        } else if ("turbidity" == n.name) {
            turbidity_ = json::read_float(n.value);
        }
    }

    update();
}

Model& Sky::model() {
    return model_;
}

float3 Sky::sun_wi(float v) const {
    float const y = (2.f * v) - 1.f;

    float3 const ls     = float3(0.5f, y, 0.f);
    float const  radius = math::degrees_to_radians(model_.degrees());
    float3 const ws     = radius * math::transform_vector(ls, sun_rotation_);

    return math::normalize(ws - sun_rotation_.r[2]);
}

float Sky::sun_v(f_float3 wi) const {
    float3 const k  = wi - sun_rotation_.r[2];
    float3 const sk = k / math::degrees_to_radians(model_.degrees());

    return (math::dot(sun_rotation_.r[1], sk) + 1.f) * 0.5f;
}

bool Sky::sky_changed_since_last_check() {
    bool const sky_changed = sky_changed_;

    sky_changed_ = false;

    return sky_changed;
}

bool Sky::sun_changed_since_last_check() {
    bool const sun_changed = sun_changed_;

    sun_changed_ = false;

    return sun_changed;
}

void Sky::update() {
    model_.set_sun_direction(sun_rotation_.r[2]);
    model_.set_ground_albedo(ground_albedo_);
    model_.set_turbidity(turbidity_);

    math::Transformation transformation{float3::identity(),
                                        float3(math::degrees_to_radians(model_.degrees())),
                                        math::quaternion::create(sun_rotation_)};

    sun_->set_transformation(transformation);

    model_.compile();

    sky_changed_ = true;
    sun_changed_ = true;
}

void Sky::on_set_transformation() {
    if (implicit_rotation_) {
        sun_rotation_ = math::quaternion::create_matrix3x3(local_frame_a().rotation);

        update();
    }
}

}  // namespace procedural::sky
