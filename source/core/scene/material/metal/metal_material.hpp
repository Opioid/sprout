#pragma once

#include "scene/material/material.hpp"
#include "scene/material/material_sample_cache.hpp"
#include "image/texture/texture_2d.hpp"

namespace scene { namespace material { namespace metal {

class Sample_isotropic;

class Material_isotropic : public material::Material<Generic_sample_cache<Sample_isotropic>> {
public:

	Material_isotropic(Generic_sample_cache<Sample_isotropic>& cache,
					   std::shared_ptr<image::texture::Texture_2D> mask, bool two_sided);

	virtual const material::Sample& sample(const shape::Differential& dg, const math::vec3& wo,
										   float time, float ior_i,
										   const image::texture::sampler::Sampler_2D& sampler,
										   uint32_t worker_id) final override;

	virtual math::vec3 sample_emission(math::float2 uv, float time,
										 const image::texture::sampler::Sampler_2D& sampler) const final override;

	virtual math::vec3 average_emission() const final override;

	virtual bool has_emission_map() const final override;

	void set_normal_map(std::shared_ptr<image::texture::Texture_2D> normal_map);

	void set_ior(const math::vec3& ior);
	void set_absorption(const math::vec3& absorption);
	void set_roughness(float roughness);

protected:

	std::shared_ptr<image::texture::Texture_2D> normal_map_;

	math::vec3 ior_;
	math::vec3 absorption_;

	float roughness_;
};

class Sample_anisotropic;

class Material_anisotropic : public material::Material<Generic_sample_cache<Sample_anisotropic>> {
public:

	Material_anisotropic(Generic_sample_cache<Sample_anisotropic>& cache,
						 std::shared_ptr<image::texture::Texture_2D> mask, bool two_sided);

	virtual const material::Sample& sample(const shape::Differential& dg, const math::vec3& wo,
										   float time, float ior_i,
										   const image::texture::sampler::Sampler_2D& sampler,
										   uint32_t worker_id) final override;

	virtual math::vec3 sample_emission(math::float2 uv, float time,
										 const image::texture::sampler::Sampler_2D& sampler) const final override;

	virtual math::vec3 average_emission() const final override;

	virtual bool has_emission_map() const final override;

	void set_normal_map(std::shared_ptr<image::texture::Texture_2D> normal_map);
	void set_direction_map(std::shared_ptr<image::texture::Texture_2D> direction_map);

	void set_ior(const math::vec3& ior);
	void set_absorption(const math::vec3& absorption);
	void set_roughness(math::float2 roughness);

protected:

	std::shared_ptr<image::texture::Texture_2D> normal_map_;
	std::shared_ptr<image::texture::Texture_2D> direction_map_;

	math::vec3 ior_;
	math::vec3 absorption_;

	math::float2 roughness_;
};

}}}

