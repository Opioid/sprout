#pragma once

#include "sky_model.hpp"
#include "core/scene/entity/entity.hpp"
#include "base/json/json_types.hpp"

namespace scene { class Prop; }

namespace procedural { namespace sky {

class Sky : public scene::entity::Entity {

public:

	Sky();
	~Sky();

	virtual void set_parameters(const json::Value& parameters) override final;

	void init(scene::Prop* sky, scene::Prop* sun);

	Model& model();

private:

	void update();

	virtual void on_set_transformation() override final;

	Model model_;

	scene::Prop* sky_;
	scene::Prop* sun_;

	float3x3 sun_rotation_;
	float3 ground_albedo_;
	float turbidity_;
};

}}
