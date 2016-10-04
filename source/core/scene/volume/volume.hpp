#pragma once

#include "scene/entity/entity.hpp"
#include "base/math/bounding/aabb.hpp"
#include "base/math/ray.hpp"

namespace scene { namespace volume {

class Volume : public entity::Entity {

public:

	Volume();

	virtual float3 optical_depth(const math::Oray& ray) const = 0;

	virtual float3 scattering() const = 0;

	virtual float phase(float3_p w, float3_p wp) const = 0;

	virtual void set_parameters(const json::Value& parameters) final override;

	void set_scene_aabb(const math::aabb& aabb);

private:

	virtual void on_set_transformation() final override;

protected:

	math::aabb scene_bb_;

	float3 absorption_;
	float3 scattering_;

	float anisotropy_;
};

}}
