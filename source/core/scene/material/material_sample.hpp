#pragma once

#include "base/math/vector3.hpp"

namespace sampler { class Sampler; }

namespace scene { namespace material {

namespace bxdf { struct Result; }

class Sample {

public:

	struct Layer {
		void set_tangent_frame(const float3& t, const float3& b, const float3& n);
		void set_tangent_frame(const float3& n);

		float clamped_n_dot(const float3& v) const;
		float reversed_clamped_n_dot(const float3& v) const;

		const float3& shading_normal() const;

		float3 tangent_to_world(const float3& v) const;

		float3 t_;
		float3 b_;
		float3 n_;
	};

	virtual const Layer& base_layer() const = 0;

	virtual float3 evaluate(const float3& wi, float& pdf) const = 0;

	virtual void sample(sampler::Sampler& sampler, bxdf::Result& result) const = 0;

	virtual float3 radiance() const = 0;

	virtual float3 attenuation() const = 0;

	virtual float ior() const = 0;

	virtual bool is_pure_emissive() const = 0;

	virtual bool is_transmissive() const = 0;

	virtual bool is_translucent() const = 0;

	float clamped_geo_n_dot(const float3& v) const;
	float reversed_clamped_geo_n_dot(const float3& v) const;

	const float3& geometric_normal() const;

	bool same_hemisphere(const float3& v) const;

	void set_basis(const float3& geo_n, const float3& wo);

	static float3 attenuation(const float3& color, float distance);

	static float clamped_dot(const float3& a, const float3& b);

	static float reversed_clamped_dot(const float3& a, const float3& b);

	static float absolute_clamped_dot(const float3& a, const float3& b);

// protected:

	float3 geo_n_;
	float3 wo_;
};

}}
