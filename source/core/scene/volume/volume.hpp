#pragma once

#include "scene/entity/entity.hpp"
#include "base/math/bounding/aabb.hpp"
#include "base/math/ray.hpp"

namespace scene { namespace volume {

class Volume : public entity::Entity {

public:

	virtual math::float3 optical_depth(const math::Oray& ray) const = 0;

	virtual math::float3 scattering() const = 0;

	virtual float phase(const math::float3& w, const math::float3& wp) const = 0;

	virtual void set_parameters(const json::Value& parameters) final override;

	void set_scene_aabb(const math::aabb& aabb);

protected:

	math::aabb scene_bb_;

private:

	virtual void on_set_transformation() final override;
};

}}
