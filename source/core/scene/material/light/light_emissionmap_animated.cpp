#include "light_emissionmap_animated.hpp"
#include "light_material_sample.hpp"
#include "image/texture/texture_adapter.inl"
#include "scene/scene_renderstate.inl"
#include "scene/scene_worker.hpp"
#include "scene/material/material_sample.inl"
#include "scene/material/material_sample_cache.inl"
#include "scene/shape/shape.hpp"
#include "base/spectrum/rgb.inl"
#include "base/math/math.hpp"
#include "base/math/distribution/distribution_2d.inl"

namespace scene { namespace material { namespace light {

Emissionmap_animated::Emissionmap_animated(Sample_cache<Sample>& cache,
										   const Sampler_settings& sampler_settings,
										   bool two_sided, const Texture_adapter& emission_map,
										   float emission_factor, float animation_duration) :
	Material(cache, sampler_settings, two_sided),
	emission_map_(emission_map),
	emission_factor_(emission_factor),
	average_emission_(float3(-1.f)),
	frame_length_(animation_duration / static_cast<float>(emission_map_.texture()->num_elements())),
	element_(0) {}

void Emissionmap_animated::tick(float absolute_time, float /*time_slice*/) {
	int32_t element = static_cast<int32_t>(absolute_time / frame_length_) %
										   emission_map_.texture()->num_elements();

	if (element != element_) {
		element_ = element;
		average_emission_ = float3(-1.f);
	}
}

const material::Sample& Emissionmap_animated::sample(float3_p wo, const Renderstate& rs,
													 const Worker& worker, Sampler_filter filter) {
	auto& sample = cache_.get(worker.id());

	auto& sampler = worker.sampler_2D(sampler_key(), filter);

	sample.set_basis(rs.geo_n, wo);

	sample.layer_.set_tangent_frame(rs.t, rs.b, rs.n);

	float3 radiance = emission_map_.sample_3(sampler, rs.uv, element_);
	sample.layer_.set(emission_factor_ * radiance);

	return sample;
}

float3 Emissionmap_animated::sample_radiance(float3_p /*wi*/, float2 uv, float /*area*/,
											 float /*time*/, const Worker& worker,
											 Sampler_filter filter) const {
	auto& sampler = worker.sampler_2D(sampler_key(), filter);
	return emission_factor_ * emission_map_.sample_3(sampler, uv, element_);
}

float3 Emissionmap_animated::average_radiance(float /*area*/) const {
	return average_emission_;
}

bool Emissionmap_animated::has_emission_map() const {
	return emission_map_.is_valid();
}

float2 Emissionmap_animated::radiance_sample(float2 r2, float& pdf) const {
	float2 uv = distribution_.sample_continuous(r2, pdf);

	pdf *= total_weight_;

	return uv;
}

float Emissionmap_animated::emission_pdf(float2 uv, const Worker& worker,
										 Sampler_filter filter) const {
	auto& sampler = worker.sampler_2D(sampler_key(), filter);

	return distribution_.pdf(sampler.address(uv)) * total_weight_;
}

float Emissionmap_animated::opacity(float2 uv, float /*time*/,
									const Worker& worker, Sampler_filter filter) const {
	if (mask_.is_valid()) {
		auto& sampler = worker.sampler_2D(sampler_key(), filter);
		return mask_.sample_1(sampler, uv, element_);
	} else {
		return 1.f;
	}
}

void Emissionmap_animated::prepare_sampling(const shape::Shape& shape, uint32_t /*part*/,
											const Transformation& /*transformation*/,
											float /*area*/, bool importance_sampling,
											thread::Pool& /*pool*/) {
	if (average_emission_.x >= 0.f) {
		// Hacky way to check whether prepare_sampling has been called before
		// average_emission_ is initialized with negative values...
		return;
	}

	if (importance_sampling) {
		float3 average_radiance = float3(0.f);

		float total_weight = 0.f;

		auto d = emission_map_.texture()->dimensions_2();
		std::vector<float> luminance(d.x * d.y);

		float2 id(1.f / static_cast<float>(d.x), 1.f / static_cast<float>(d.y));

		auto texture = emission_map_.texture();

		for (int32_t y = 0, l = 0; y < d.y; ++y) {
			float v = id.y * (static_cast<float>(y) + 0.5f);

			for (int32_t x = 0; x < d.x; ++x, ++l) {
				float u = id.x * (static_cast<float>(x) + 0.5f);

				float uv_weight = shape.uv_weight(float2(u, v));

				float3 radiance = emission_factor_ * texture->at_element_3(x, y, element_);

				average_radiance += uv_weight * radiance;

				total_weight += uv_weight;

				luminance[l] = uv_weight * spectrum::luminance(radiance);
			}
		}

		average_emission_ = average_radiance / total_weight;

		total_weight_ = total_weight;

		distribution_.init(luminance.data(), d);
	} else {
		average_emission_ = emission_factor_ * emission_map_.texture()->average_3();
	}

	if (is_two_sided()) {
		average_emission_ *= 2.f;
	}
}

bool Emissionmap_animated::is_animated() const {
	return true;
}

size_t Emissionmap_animated::num_bytes() const {
	return sizeof(*this);
}

}}}
