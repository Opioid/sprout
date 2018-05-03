#ifndef SU_EXTENSION_PROCEDURAL_SKY_HPP
#define SU_EXTENSION_PROCEDURAL_SKY_HPP

#include "sky_model.hpp"
#include "core/scene/entity/entity.hpp"

namespace scene::prop { class Prop; }

namespace procedural::sky {

class Sky : public scene::entity::Entity {

public:

	Sky() = default;
	~Sky();

	virtual void set_parameters(json::Value const& parameters) override final;

	void init(scene::prop::Prop* sky, scene::prop::Prop* sun);

	Model& model();

private:

	void update();

	virtual void on_set_transformation() override final;

	Model model_;

	scene::prop::Prop* sky_;
	scene::prop::Prop* sun_;

	float3x3 sun_rotation_ = float3x3(1.f,  0.f, 0.f,
									  0.f,  0.f, 1.f,
									  0.f, -1.f, 0.f);

	float3 ground_albedo_ = float3(0.2f, 0.2f, 0.2f);

	float turbidity_ = 2.f;

	bool implicit_rotation_ = true;
};

}

#endif
