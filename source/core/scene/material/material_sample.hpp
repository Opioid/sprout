#pragma once

#include "base/math/vector.hpp"

namespace sampler { class Sampler; }

namespace scene { namespace material {

namespace bxdf { struct Result; }

class Sample {

public:

	struct Layer {
		void set_tangent_frame(float3_p t, float3_p b, float3_p n);
		void set_tangent_frame(float3_p n);

		float clamped_n_dot(float3_p v) const;
		float reversed_clamped_n_dot(float3_p v) const;

		float3_p shading_normal() const;

		float3 tangent_to_world(float3_p v) const;

		float3 t_;
		float3 b_;
		float3 n_;
	};

	virtual const Layer& base_layer() const = 0;

	virtual float3 evaluate(float3_p wi, float& pdf) const = 0;

	virtual void sample(sampler::Sampler& sampler, bxdf::Result& result) const = 0;

	virtual float3 radiance() const = 0;

	virtual float3 attenuation() const = 0;

	virtual float ior() const = 0;

	virtual bool is_pure_emissive() const = 0;

	virtual bool is_transmissive() const = 0;

	virtual bool is_translucent() const = 0;

	float clamped_geo_n_dot(float3_p v) const;
	float reversed_clamped_geo_n_dot(float3_p v) const;

	float3_p geometric_normal() const;

	bool same_hemisphere(float3_p v) const;

	void set_basis(float3_p geo_n, float3_p wo);

	static float3 attenuation(float3_p color, float distance);

	static float clamped_dot(float3_p a, float3_p b);

	static float reversed_clamped_dot(float3_p a, float3_p b);

// protected:

	float3 geo_n_;
	float3 wo_;
};

}}
