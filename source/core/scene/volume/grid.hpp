#ifndef SU_CORE_SCENE_VOLUME_GRID_HPP
#define SU_CORE_SCENE_VOLUME_GRID_HPP

#include "density.hpp"
#include "image/texture/texture_adapter.hpp"
#include "image/texture/texture_types.hpp"

namespace scene::volume {

class Grid : public Density {

public:

	Grid(const Texture_ptr& grid);

private:

	virtual float density(const float3& p, Sampler_filter filter,
						  const Worker& worker) const override final;

	virtual void set_parameter(const std::string& name, const json::Value& value) override final;

	Texture_adapter grid_;
};

class Emission_grid : public Volume {

public:

	Emission_grid(const Texture_ptr& grid);

	virtual float3 emission(const math::Ray& ray, float step_size, rnd::Generator& rng,
							Sampler_filter filter, const Worker& worker) const override final;

	virtual float3 optical_depth(const math::Ray& ray, float step_size, rnd::Generator& rng,
								 Sampler_filter filter, const Worker& worker) const override final;

	virtual float3 scattering(const float3& p, Sampler_filter filter,
							  const Worker& worker) const override final;

private:

	float3 emission(const float3& p, Sampler_filter filter, const Worker& worker) const;

	virtual void set_parameter(const std::string& name, const json::Value& value) override final;

	Texture_adapter grid_;
};

}

#endif
