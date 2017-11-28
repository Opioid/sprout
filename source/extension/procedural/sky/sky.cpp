#include "sky.hpp"
#include "core/scene/prop/prop.hpp"
#include "base/json/json.hpp"
#include "base/math/vector3.inl"
#include "base/math/matrix3x3.inl"
#include "base/math/matrix4x4.inl"
#include "base/math/quaternion.inl"

namespace procedural { namespace sky {

Sky::~Sky() {}

void Sky::init(scene::prop::Prop* sky, scene::prop::Prop* sun) {
	sky_ = sky;
	sun_ = sun;

//	attach(sky_);
//	attach(sun_);

	update();

	const math::Transformation transformation {
		float3::identity(),
		float3(1.f),
		math::quaternion::create_rotation_x(math::degrees_to_radians(90.f))
	};

	sky->set_transformation(transformation);
}

void Sky::set_parameters(const json::Value& parameters) {
	for (auto& n : parameters.GetObject()) {
		if ("sun" == n.name) {
			const float3 angles = json::read_float3(n.value, "rotation", float3::identity());
			sun_rotation_ = json::create_rotation_matrix(angles);
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

void Sky::update() {
	model_.set_sun_direction(sun_rotation_.r[2]);
	model_.set_ground_albedo(ground_albedo_);
	model_.set_turbidity(turbidity_);

	math::Transformation transformation {
		float3::identity(),
		float3(math::degrees_to_radians(0.255f)), // 0.255 to better match hosek
		math::quaternion::create(sun_rotation_)
	};

	sun_->set_transformation(transformation);
}

void Sky::on_set_transformation() {
	if (implicit_rotation_) {
		sun_rotation_ = math::quaternion::create_matrix3x3(local_frame_a().rotation);

		update();
	}
}

}}
