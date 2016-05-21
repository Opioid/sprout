#pragma once

#include "sky_model.hpp"
#include "core/scene/entity/entity.hpp"
#include "base/json/json_types.hpp"

namespace scene { class Prop; }

namespace procedural { namespace sky {

class Sky : public scene::entity::Entity {

public:

	Sky(scene::Prop& sky, scene::Prop& sun);
	~Sky();

	virtual void set_parameters(const json::Value& parameters) final override;

	void set_sun(scene::Prop* sun);

	Model& model();

private:

	void update();

	virtual void on_set_transformation() final override;

	Model model_;

	scene::Prop& sky_;
	scene::Prop& sun_;

	math::float3x3 sun_rotation_;
	math::float3 ground_albedo_;
	float turbidity_;
};

}}
