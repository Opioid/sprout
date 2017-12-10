#ifndef SU_CORE_SCENE_VOLUME_VOLUME_HPP
#define SU_CORE_SCENE_VOLUME_VOLUME_HPP

#include "scene/body/body.hpp"
#include "scene/material/sampler_settings.hpp"
#include "base/math/aabb.hpp"
#include "base/math/ray.hpp"
#include <string>

namespace rnd { class Generator; }

namespace scene {

class Worker;

namespace volume {

class Volume : public body::Body {

public:

	using Sampler_filter = material::Sampler_settings::Filter;

	Volume();

	virtual float3 emission(const Transformation& transformation, const math::Ray& ray,
							float step_size, rnd::Generator& rng,
							Sampler_filter filter, const Worker& worker) const = 0;

	virtual float3 optical_depth(const Transformation& transformation, const math::Ray& ray,
								 float step_size, rnd::Generator& rng,
								 Sampler_filter filter, const Worker& worker) const = 0;

	virtual float3 scattering(const Transformation& transformation, const float3& p,
							  Sampler_filter filter, const Worker& worker) const = 0;

	float phase(const float3& w, const float3& wp) const;

	virtual void set_parameters(const json::Value& parameters) override final;

	void set_scene_aabb(const math::AABB& aabb);

protected:

	virtual void set_parameter(const std::string& name, const json::Value& value) = 0;

	static float phase_schlick(const float3& w, const float3& wp, float k);

	float3 absorption_;
	float3 scattering_;

	float anisotropy_;

	bool match_scene_scale_;
};

}}

#endif
