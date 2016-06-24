#pragma once

#include "sampler_settings.hpp"
#include "image/texture/texture_types.hpp"
#include "base/json/json_types.hpp"
#include "base/math/vector.hpp"
#include <memory>
#include <vector>

namespace scene {

class Worker;
struct Renderstate;

namespace shape { struct Hitpoint; }

namespace material {

class Sample;

class Material {

public:

	using Sampler_filter = material::Sampler_settings::Filter;

	Material(Texture_2D_ptr mask, const Sampler_settings& sampler_settings, bool two_sided);

	virtual ~Material();

	void set_parameters(const json::Value& parameters);

	virtual void tick(float absolute_time, float time_slice);

	virtual const Sample& sample(float3_p wo, const Renderstate& rs,
								 const Worker& worker, Sampler_filter filter) = 0;

	virtual float3 sample_radiance(float3_p wi, float2 uv, float area, float time,
								   const Worker& worker, Sampler_filter filter) const;

	virtual float3 average_radiance(float area) const;

	virtual bool has_emission_map() const;

	virtual float2 radiance_importance_sample(float2 r2, float& pdf) const;

	virtual float emission_pdf(float2 uv, const Worker& worker, Sampler_filter filter) const;

	virtual float opacity(float2 uv, float time, const Worker& worker,
						  Sampler_filter filter) const;

	virtual void prepare_sampling(bool spherical);

	virtual bool is_animated() const;

	uint32_t sampler_key() const;

	bool is_masked() const;
	bool is_emissive() const;
	bool is_two_sided() const;

protected:

	virtual void set_parameter(const std::string& name,
							   const json::Value& value);

	Texture_2D_ptr mask_;

	uint32_t sampler_key_;

	bool two_sided_;
};

template<typename Sample_cache>
class Typed_material : public Material {

public:

	Typed_material(Sample_cache& cache, Texture_2D_ptr mask,
				   const Sampler_settings& sampler_settings, bool two_sided) :
		Material(mask, sampler_settings, two_sided), cache_(cache) {}

protected:

	Sample_cache& cache_;
};

using Materials = std::vector<std::shared_ptr<material::Material>>;

}}
