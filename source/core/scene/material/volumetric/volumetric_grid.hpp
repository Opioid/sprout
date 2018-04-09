#ifndef SU_CORE_SCENE_MATERIAL_VOLUMETRIC_GRID_HPP
#define SU_CORE_SCENE_MATERIAL_VOLUMETRIC_GRID_HPP

#include "volumetric_density.hpp"
#include "image/texture/texture_adapter.hpp"

namespace scene::material::volumetric {

class Grid final : public Density {

public:

	Grid(const Sampler_settings& sampler_settings, const Texture_adapter& grid);

	~Grid();

	virtual void compile() override final;

	virtual float3 max_extinction(float2 uv, Sampler_filter filter,
								  const Worker& worker) const override final;

	virtual bool  is_heterogeneous_volume() const override final;

	virtual size_t num_bytes() const override final;

private:

	virtual float density(const Transformation& transformation, const float3& p,
						  Sampler_filter filter, const Worker& worker) const override final;

	Texture_adapter grid_;

	float3 max_extinction_;
};

class Emission_grid final : public Material {

public:

	Emission_grid(const Sampler_settings& sampler_settings, const Texture_adapter& grid);

	~Emission_grid();

	virtual float3 emission(const Transformation& transformation, const math::Ray& ray,
							float step_size, rnd::Generator& rng,
							Sampler_filter filter, const Worker& worker) const override final;

	virtual float3 optical_depth(const Transformation& transformation, const math::AABB& aabb,
								 const math::Ray& ray, float step_size, rnd::Generator& rng,
								 Sampler_filter filter, const Worker& worker) const override final;

	virtual float3 scattering(const Transformation& transformation, const float3& p, float2 uv,
							  Sampler_filter filter, const Worker& worker) const override final;

	virtual size_t num_bytes() const override final;

private:

	float3 emission(const float3& p, Sampler_filter filter, const Worker& worker) const;

	Texture_adapter grid_;
};

}

#endif
