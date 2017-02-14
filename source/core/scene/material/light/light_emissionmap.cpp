#include "light_emissionmap.hpp"
#include "light_material_sample.hpp"
#include "image/texture/texture_adapter.inl"
#include "scene/scene_renderstate.inl"
#include "scene/scene_worker.hpp"
#include "scene/material/material_sample.inl"
#include "scene/material/material_sample_cache.inl"
#include "scene/shape/shape.hpp"
#include "base/math/math.hpp"
#include "base/math/distribution/distribution_2d.inl"
#include "base/spectrum/rgb.inl"
#include "base/thread/thread_pool.hpp"

namespace scene { namespace material { namespace light {

Emissionmap::Emissionmap(Sample_cache& sample_cache, const Sampler_settings& sampler_settings,
						 bool two_sided) :
	Material(sample_cache, sampler_settings, two_sided),
	average_emission_(float3(-1.f)) {}

const material::Sample& Emissionmap::sample(float3_p wo, const Renderstate& rs,
											const Worker& worker, Sampler_filter filter) {
	auto& sample = sample_cache_.get<Sample>(worker.id());

	auto& sampler = worker.sampler_2D(sampler_key(), filter);

	sample.set_basis(rs.geo_n, wo);

	sample.layer_.set_tangent_frame(rs.t, rs.b, rs.n);

	float3 radiance = emission_map_.sample_3(sampler, rs.uv);
	sample.layer_.set(emission_factor_ * radiance);

	return sample;
}

float3 Emissionmap::sample_radiance(float3_p /*wi*/, float2 uv, float /*area*/,
									float /*time*/, const Worker& worker,
									Sampler_filter filter) const {
	auto& sampler = worker.sampler_2D(sampler_key(), filter);
	return emission_factor_ * emission_map_.sample_3(sampler, uv);
}

float3 Emissionmap::average_radiance(float /*area*/) const {
	return average_emission_;
}

bool Emissionmap::has_emission_map() const {
	return emission_map_.is_valid();
}

float2 Emissionmap::radiance_sample(float2 r2, float& pdf) const {
	float2 uv = distribution_.sample_continuous(r2, pdf);

	pdf *= total_weight_;

	return uv;
}

float Emissionmap::emission_pdf(float2 uv, const Worker& worker,
								Sampler_filter filter) const {
	auto& sampler = worker.sampler_2D(sampler_key(), filter);

	return distribution_.pdf(sampler.address(uv)) * total_weight_;
}

void Emissionmap::prepare_sampling(const shape::Shape& shape, uint32_t /*part*/,
								   const Transformation& /*transformation*/,
								   float /*area*/, bool importance_sampling,
								   thread::Pool& pool) {
	if (average_emission_.x >= 0.f) {
		// Hacky way to check whether prepare_sampling has been called before
		// average_emission_ is initialized with negative values...
		return;
	}

	if (importance_sampling) {
		auto d = emission_map_.texture()->dimensions_2();
		std::vector<float> luminance(d.x * d.y);

		float2 rd(1.f / static_cast<float>(d.x), 1.f / static_cast<float>(d.y));

		auto texture = emission_map_.texture();

		float ef = emission_factor_;

		std::vector<float4> artws(pool.num_threads(), float4(0.f));

		pool.run_range([&luminance, &artws, &shape, texture, d, rd, ef]
			(uint32_t id, int32_t begin, int32_t end) {
				for (int32_t y = begin; y < end; ++y) {
					float v = rd.y * (static_cast<float>(y) + 0.5f);

					for (int32_t x = 0; x < d.x; ++x) {
						float u = rd.x * (static_cast<float>(x) + 0.5f);

						float uv_weight = shape.uv_weight(float2(u, v));

						float3 radiance = ef * texture->at_3(x, y);

						int32_t l = y * d.x + x;
						luminance[l] = uv_weight * spectrum::luminance(radiance);

						artws[id] += float4(uv_weight * radiance, uv_weight);
					}
				}
			}, 0, d.y);

		// artw: (float3(averave_radiance), total_weight)
		float4 artw(0.f);
		for (auto& a : artws) {
			artw += a;
		}

		average_emission_ = artw.xyz / artw.w;

		total_weight_ = artw.w;

		distribution_.init(luminance.data(), d, pool);
	} else {
		average_emission_ = emission_factor_ * emission_map_.texture()->average_3();
	}

	if (is_two_sided()) {
		average_emission_ *= 2.f;
	}
}

size_t Emissionmap::num_bytes() const {
	return sizeof(*this) + distribution_.num_bytes();
}

void Emissionmap::set_emission_map(const Texture_adapter& emission_map) {
	emission_map_ = emission_map;
}

void Emissionmap::set_emission_factor(float emission_factor) {
	emission_factor_ = emission_factor;
}

}}}
