#ifndef SU_CORE_SCENE_MATERIAL_MATERIAL_HPP
#define SU_CORE_SCENE_MATERIAL_MATERIAL_HPP

#include "sampler_settings.hpp"
#include "image/texture/texture_adapter.hpp"
#include "image/texture/texture_types.hpp"
#include "base/json/json_types.hpp"
#include "base/math/vector3.hpp"
#include "base/spectrum/discrete.hpp"
#include <memory>
#include <vector>

namespace math { struct Ray; class AABB; }

namespace rnd { class Generator; }

namespace sampler { class Sampler; }

namespace thread { class Pool; }

namespace scene {

struct Renderstate;
class Worker;

namespace entity { struct Composed_transformation; }

namespace shape { class Shape; }

namespace material {

class Sample;

class Material {

public:

	using Transformation = entity::Composed_transformation;
	using Sampler_filter = Sampler_settings::Filter;

	Material(const Sampler_settings& sampler_settings, bool two_sided);

	virtual ~Material();

	void set_mask(const Texture_adapter& mask);

	void set_parameters(json::Value const& parameters);

	virtual void compile();

	virtual void tick(float absolute_time, float time_slice);

	virtual const Sample& sample(f_float3 wo, const Renderstate& rs, Sampler_filter filter,
								 sampler::Sampler& sampler, const Worker& worker) const = 0;

	virtual float3 sample_radiance(f_float3 wi, float2 uv, float area, float time,
								   Sampler_filter filter, const Worker& worker) const;

	virtual float3 average_radiance(float area) const;

	virtual bool has_emission_map() const;

	struct Sample_2D { float2 uv; float pdf; };
	virtual Sample_2D radiance_sample(float2 r2) const;

	virtual float emission_pdf(float2 uv, Sampler_filter filter, const Worker& worker) const;

	virtual float opacity(float2 uv, float time, Sampler_filter filter, const Worker& worker) const;

	virtual float3 thin_absorption(f_float3 wo, f_float3 n, float2 uv, float time,
								   Sampler_filter filter, const Worker& worker) const;

	virtual float3 emission( const math::Ray& ray, const Transformation& transformation,
							float step_size, rnd::Generator& rng,
							Sampler_filter filter, const Worker& worker) const;

	virtual float3 absorption_coefficient(float2 uv, Sampler_filter filter,
										  const Worker& worker) const;

	virtual void collision_coefficients(float2 uv, Sampler_filter filter, const Worker& worker,
										float3& mu_a, float3& mu_s) const;

	virtual void collision_coefficients(f_float3 p, const Transformation& transformation,
										Sampler_filter filter, const Worker& worker,
										float3& mu_a, float3& mu_s) const;

	virtual float majorant_mu_t() const;

	virtual bool is_heterogeneous_volume() const;
	virtual bool is_scattering_volume() const;

	virtual void prepare_sampling(const shape::Shape& shape, uint32_t part,
								  const Transformation& transformation, float area,
								  bool importance_sampling, thread::Pool& pool);

	virtual bool is_animated() const;

	virtual bool has_tinted_shadow() const;

	virtual float ior() const = 0;

	uint32_t sampler_key() const;

	virtual bool is_masked() const;
	bool is_emissive() const;
	bool is_two_sided() const;

	virtual size_t num_bytes() const = 0;

protected:

	virtual void set_parameter(std::string const& name, json::Value const& value);

private:

	uint32_t sampler_key_;

	bool two_sided_;

protected:

	Texture_adapter mask_;

public:

	static void init_rainbow();

	static float3 spectrum_at_wavelength(float lambda, float value = 1.f);

	static constexpr int32_t Num_bands = 16;

	using Spectrum = spectrum::Discrete_spectral_power_distribution<Num_bands>;

private:

	static float3 rainbow_[Num_bands];
};

}

using Material_ptr = std::shared_ptr<material::Material>;
using Materials = std::vector<Material_ptr>;

}

#endif
