#pragma once

#include "scene/entity/entity.hpp"
#include "base/math/bounding/aabb.hpp"
#include "base/math/ray.hpp"
#include <string>

namespace math { namespace random { class Generator; }}

namespace scene { namespace volume {

class Volume : public entity::Entity {

public:

	Volume();

	virtual float3 optical_depth(const math::Oray& ray, float step_size,
								 math::random::Generator& rng) const = 0;

	virtual float3 scattering(float3_p p) const = 0;

	float phase(float3_p w, float3_p wp) const;

	virtual void set_parameters(const json::Value& parameters) final override;

	void set_scene_aabb(const math::aabb& aabb);

private:

	virtual void on_set_transformation() final override;

protected:

	virtual void set_parameter(const std::string& name, const json::Value& value) = 0;

	static float phase_schlick(float3_p w, float3_p wp, float k);

	math::aabb scene_bb_;

	float3 absorption_;
	float3 scattering_;

	float anisotropy_;
};

}}
