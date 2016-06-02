#include "light_emissionmap.hpp"
#include "light_material_sample.hpp"
#include "scene/scene_worker.hpp"
#include "scene/material/material_sample.inl"
#include "scene/material/material_sample_cache.inl"
#include "scene/shape/geometry/hitpoint.inl"
#include "image/texture/sampler/sampler_2d.hpp"
#include "base/spectrum/rgb.inl"
#include "base/math/math.hpp"
#include "base/math/distribution/distribution_2d.inl"

namespace scene { namespace material { namespace light {

Emissionmap::Emissionmap(Generic_sample_cache<Sample>& cache,
						 std::shared_ptr<image::texture::Texture_2D> mask,
						 const Sampler_settings& sampler_settings, bool two_sided) :
	Material(cache, mask, sampler_settings, two_sided),
	average_emission_(math::float3(-1.f, -1.f, -1.f)) {}

const material::Sample& Emissionmap::sample(const shape::Hitpoint& hp, math::pfloat3 wo,
											float /*area*/, float /*time*/, float /*ior_i*/,
											const Worker& worker, Sampler_filter filter) {
	auto& sample = cache_.get(worker.id());

	auto& sampler = worker.sampler(sampler_key_, filter);

	sample.set_basis(hp.t, hp.b, hp.n, hp.geo_n, wo, two_sided_);

	math::float3 radiance = sampler.sample_3(*emission_map_, hp.uv);
	sample.set(radiance);

	return sample;
}

math::float3 Emissionmap::sample_radiance(math::pfloat3 /*wi*/, math::float2 uv,
										  float /*area*/, float /*time*/,
										  const Worker& worker, Sampler_filter filter) const {
	auto& sampler = worker.sampler(sampler_key_, filter);
	return emission_factor_ * sampler.sample_3(*emission_map_, uv);
}

math::float3 Emissionmap::average_radiance(float /*area*/) const {
	return average_emission_;
}

bool Emissionmap::has_emission_map() const {
	return nullptr != emission_map_;
}

math::float2 Emissionmap::radiance_importance_sample(math::float2 r2, float& pdf) const {
	math::float2 uv = distribution_.sample_continuous(r2, pdf);

	if (uv.y == 0.f) {
		pdf = 0.f;
	} else {
		float sin_theta = std::sin(uv.y * math::Pi);
/*
		math::float2 disk(
					2.f * uv.x - 1.f,
					2.f * uv.y - 1.f);


		float r = std::sqrt(disk.x * disk.x + disk.y * disk.y);

		// Equidistant projection
		float colatitude = r * math::Pi_div_2;


		float sin_theta = r * r;// std::cos(colatitude);

		if (r > 1.f) {
			pdf = 0.f;
			return uv;
		}
*/

		pdf *= total_weight_ / sin_theta;
	}

	return uv;
}

float Emissionmap::emission_pdf(math::float2 uv, const Worker& worker,
								Sampler_filter filter) const {
	if (uv.y == 0.f) {
		return 0.f;
	}

	auto& sampler = worker.sampler(sampler_key_, filter);

	float sin_theta = std::sin(uv.y * math::Pi);

	return distribution_.pdf(sampler.address(uv)) * (total_weight_ / sin_theta);
}

void Emissionmap::prepare_sampling(bool spherical) {
	if (average_emission_.x >= 0.f) {
		// Hacky way to check whether prepare_sampling has been called before
		// average_emission_ is initialized with negative values...
		return;
	}

	if (spherical) {
		math::float3 average_radiance = math::float3_identity;

		float total_weight = 0.f;

		auto d = emission_map_->dimensions();
		std::vector<float> luminance(d.x * d.y);

		float my = 1.f / static_cast<float>(d.y) * math::Pi;

		for (int32_t y = 0, l = 0; y < d.y; ++y) {
			float sin_theta = std::sin((static_cast<float>(y) + 0.5f) * my);

			for (int32_t x = 0; x < d.x; ++x, ++l) {
				math::float3 radiance = emission_factor_ * emission_map_->at_3(x, y);


			/*
				math::float2 disk(
							2.f * ((static_cast<float>(x) + 0.5f) / static_cast<float>(d.x)) - 1.f,
							2.f * ((static_cast<float>(y) + 0.5f) / static_cast<float>(d.y)) - 1.f);


				float r = std::sqrt(disk.x * disk.x + disk.y * disk.y);

				// Equidistant projection
				float colatitude = r * math::Pi_div_2;



				float sin_theta = r * r;// std::cos(colatitude);


				if (r > 1.f) {
					sin_theta = 0.f;
				}
			*/

				average_radiance += sin_theta * radiance;

				total_weight += sin_theta;

				luminance[l] = sin_theta * spectrum::luminance(radiance);
			}
		}

		average_emission_ = average_radiance / total_weight;

		total_weight_ = total_weight;

		distribution_.init(luminance.data(), d);
	} else {
		average_emission_ = emission_factor_ * emission_map_->average_3();
	}
}

void Emissionmap::set_emission_map(std::shared_ptr<image::texture::Texture_2D> emission_map) {
	emission_map_ = emission_map;
}

void Emissionmap::set_emission_factor(float emission_factor) {
	emission_factor_ = emission_factor;
}

}}}
