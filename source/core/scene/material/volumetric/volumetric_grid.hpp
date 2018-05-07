#ifndef SU_CORE_SCENE_MATERIAL_VOLUMETRIC_GRID_HPP
#define SU_CORE_SCENE_MATERIAL_VOLUMETRIC_GRID_HPP

#include "volumetric_density.hpp"
#include "volumetric_octree.hpp"
#include "image/texture/texture_adapter.hpp"

namespace scene::material::volumetric {

class Grid final : public Density {

public:

	Grid(Sampler_settings const& sampler_settings, Texture_adapter const& grid);

	virtual ~Grid() override final;

	virtual void compile() override final;

	virtual float majorant_mu_t() const override final;

	virtual Octree const* volume_octree() const override final;

	virtual bool  is_heterogeneous_volume() const override final;

	virtual size_t num_bytes() const override final;

private:

	virtual float density(f_float3 p, Sampler_filter filter,
						  Worker const& worker) const override final;

	Texture_adapter grid_;

	float majorant_mu_t_;

	Octree tree_;
};

class Emission_grid final : public Material {

public:

	Emission_grid(Sampler_settings const& sampler_settings, Texture_adapter const& grid);

	virtual ~Emission_grid() override final;

	virtual float3 emission(math::Ray const& ray, Transformation const& transformation,
							float step_size, rnd::Generator& rng,
							Sampler_filter filter, Worker const& worker) const override final;

	virtual size_t num_bytes() const override final;

private:

	float3 emission(f_float3 p, Sampler_filter filter, Worker const& worker) const;

	Texture_adapter grid_;
};

class Flow_vis_grid final : public Material {

public:

	Flow_vis_grid(Sampler_settings const& sampler_settings, Texture_adapter const& grid);

	virtual ~Flow_vis_grid() override final;

	virtual float3 emission(math::Ray const& ray, Transformation const& transformation,
							float step_size, rnd::Generator& rng,
							Sampler_filter filter, Worker const& worker) const override final;

	virtual size_t num_bytes() const override final;

private:

	float density(f_float3 p, Sampler_filter filter, Worker const& worker) const;

	float3 emission(f_float3 p, Sampler_filter filter, Worker const& worker) const;

	Texture_adapter grid_;
};

}

#endif
