#ifndef SU_CORE_SCENE_MATERIAL_VOLUMETRIC_GRID_HPP
#define SU_CORE_SCENE_MATERIAL_VOLUMETRIC_GRID_HPP

#include "volumetric_density.hpp"
#include "image/texture/texture_adapter.hpp"

namespace scene::material::volumetric {

class Grid final : public Density {

public:

	Grid(const Sampler_settings& sampler_settings, const Texture_adapter& grid);

	virtual ~Grid() override final;

	virtual void compile() override final;

	virtual float majorant_mu_t() const override final;

	virtual bool  is_heterogeneous_volume() const override final;

	virtual size_t num_bytes() const override final;

private:

	virtual float density(f_float3 p, Transformation const& transformation,
						  Sampler_filter filter, const Worker& worker) const override final;

	Texture_adapter grid_;

	float majorant_mu_t_;
};

class Emission_grid final : public Material {

public:

	Emission_grid(const Sampler_settings& sampler_settings, const Texture_adapter& grid);

	virtual ~Emission_grid() override final;

	virtual float3 emission(const math::Ray& ray, Transformation const& transformation,
							float step_size, rnd::Generator& rng,
							Sampler_filter filter, const Worker& worker) const override final;

	virtual size_t num_bytes() const override final;

private:

	float3 emission(f_float3 p, Sampler_filter filter, const Worker& worker) const;

	Texture_adapter grid_;
};

class Flow_vis_grid final : public Material {

public:

	Flow_vis_grid(const Sampler_settings& sampler_settings, const Texture_adapter& grid);

	virtual ~Flow_vis_grid() override final;

	virtual float3 emission(const math::Ray& ray, Transformation const& transformation,
							float step_size, rnd::Generator& rng,
							Sampler_filter filter, const Worker& worker) const override final;

	virtual size_t num_bytes() const override final;

private:

	float density(f_float3 p, Sampler_filter filter, const Worker& worker) const;

	float3 emission(f_float3 p, Sampler_filter filter, const Worker& worker) const;

	Texture_adapter grid_;
};

}

#endif
