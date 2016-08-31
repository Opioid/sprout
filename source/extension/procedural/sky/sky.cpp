#include "sky.hpp"
#include "core/scene/prop.hpp"
#include "base/json/json.hpp"
#include "base/math/vector.inl"
#include "base/math/matrix.inl"

namespace procedural { namespace sky {

Sky::Sky() :
	sun_rotation_(1.f, 0.f, 0.f,
				  0.f, 0.f, 1.f,
				  0.f, -1.f, 0.f),
	ground_albedo_(0.2f, 0.2f, 0.2f),
	turbidity_(2.f) {}

Sky::~Sky() {}

void Sky::init(scene::Prop* sky, scene::Prop* sun) {
	sky_ = sky;
	sun_ = sun;

	attach(sky_);
	attach(sun_);

	update();

	math::transformation transformation {
		math::float3_identity,
		float3(1.f, 1.f, 1.f),
		math::create_quaternion_rotation_x(math::degrees_to_radians(90.f))
	};

	sky->set_transformation(transformation);
}

void Sky::set_parameters(const json::Value& parameters) {
	for (auto n = parameters.MemberBegin(); n != parameters.MemberEnd(); ++n) {
		const std::string node_name = n->name.GetString();
		const json::Value& node_value = n->value;

		if ("sun" == node_name) {
			float3 angles = json::read_float3(node_value, "rotation");
			sun_rotation_ = json::create_rotation_matrix(angles);
		} else if ("ground_albedo" == node_name) {
			ground_albedo_ = json::read_float3(node_value);
		} else if ("turbidity" == node_name) {
			turbidity_ = json::read_float(node_value);
		}
	}

	update();
}

Model& Sky::model() {
	return model_;
}

void Sky::update() {
	model_.set_sun_direction(float3(sun_rotation_.z));
	model_.set_ground_albedo(ground_albedo_);
	model_.set_turbidity(turbidity_);

	math::transformation transformation {
		math::float3_identity,
		float3(0.26f, 0.26f, 0.26f),
		math::create_quaternion(sun_rotation_)
	};

	sun_->set_transformation(transformation);
}

void Sky::on_set_transformation() {}

}}
