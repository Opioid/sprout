#include "sky_material.hpp"
#include "sky_model.hpp"
#include "core/image/typed_image.inl"
#include "core/image/texture/texture_adapter.inl"
#include "core/image/texture/texture_float_3.hpp"
#include "core/scene/scene_renderstate.hpp"
#include "core/scene/scene_worker.inl"
#include "core/scene/prop/prop.hpp"
#include "core/scene/shape/shape_sample.hpp"
#include "core/scene/shape/shape.hpp"
#include "core/scene/material/material_sample.inl"
#include "core/scene/material/light/light_material_sample.hpp"
#include "base/math/mapping.inl"
#include "base/math/matrix3x3.inl"
#include "base/math/vector3.inl"
#include "base/math/distribution/distribution_2d.inl"
#include "base/spectrum/rgb.hpp"

//#include "core/image/encoding/png/png_writer.hpp"
//#include <fstream>
//#include <iostream>

namespace procedural::sky {

using namespace scene;

Sky_material::Sky_material(Model& model) : Material(model) {}

const material::Sample& Sky_material::sample(const float3& wo, const Renderstate& rs,
											 Sampler_filter /*filter*/,
											 sampler::Sampler& /*sampler*/,
											 const Worker& worker) const {
	auto& sample = worker.sample<material::light::Sample>();

	sample.set_basis(rs.geo_n, wo);

	sample.layer_.set_tangent_frame(rs.t, rs.b, rs.n);

	sample.layer_.set(model_.evaluate_sky(-wo));

	return sample;
}

float3 Sky_material::sample_radiance(const float3& wi, float2 /*uv*/, float /*area*/,
									 float /*time*/, Sampler_filter /*filter*/,
									 const Worker& /*worker*/) const {
	return model_.evaluate_sky(wi);
}

float3 Sky_material::average_radiance(float /*area*/) const {
	return model_.evaluate_sky(model_.zenith());
}

void Sky_material::prepare_sampling(const shape::Shape& /*shape*/, uint32_t /*part*/,
									const Transformation& /*transformation*/,
									float /*area*/, bool /*importance_sampling*/,
									thread::Pool& /*pool*/) {
	model_.init();
}

float Sky_material::ior() const {
	return 1.5f;
}

size_t Sky_material::num_bytes() const {
	return sizeof(*this);
}

Sky_baked_material::Sky_baked_material(Model& model) : Material(model) {}

Sky_baked_material::~Sky_baked_material() {}

const material::Sample& Sky_baked_material::sample(const float3& wo, const Renderstate& rs,
												   Sampler_filter filter,
												   sampler::Sampler& /*sampler*/,
												   const Worker& worker) const {
	auto& sample = worker.sample<material::light::Sample>();

	const auto& sampler = worker.sampler_2D(sampler_key(), filter);

	sample.set_basis(rs.geo_n, wo);

	sample.layer_.set_tangent_frame(rs.t, rs.b, rs.n);

	const float3 radiance = emission_map_.sample_3(sampler, rs.uv);
	sample.layer_.set(radiance);

	return sample;
}

float3 Sky_baked_material::sample_radiance(const float3& /*wi*/, float2 uv, float /*area*/,
										   float /*time*/, Sampler_filter filter,
										   const Worker& worker) const {
	auto& sampler = worker.sampler_2D(sampler_key(), filter);
	return emission_map_.sample_3(sampler, uv);
}

float3 Sky_baked_material::average_radiance(float /*area*/) const {
	return average_emission_;
}

bool Sky_baked_material::has_emission_map() const {
	return true;
}

Material::Sample_2D Sky_baked_material::radiance_sample(float2 r2) const {
	const auto result = distribution_.sample_continuous(r2);

	return { result.uv, result.pdf * total_weight_ };
}

float Sky_baked_material::emission_pdf(float2 uv, Sampler_filter filter,
									   const Worker& worker) const {
	auto& sampler = worker.sampler_2D(sampler_key(), filter);

	return distribution_.pdf(sampler.address(uv)) * total_weight_;
}

void Sky_baked_material::prepare_sampling(const shape::Shape& shape, uint32_t /*part*/,
										  const Transformation& transformation, float /*area*/,
										  bool importance_sampling, thread::Pool& /*pool*/) {
	if (!model_.init()) {
		return;
	}

	const int2 d(256, 256);

	const image::Image::Description description(image::Image::Type::Float3, d);
	auto cache = std::make_shared<image::Float3>(description);

	for (int32_t y = 0; y < d[1]; ++y) {
		for (int32_t x = 0; x < d[0]; ++x) {
			const float2 uv((static_cast<float>(x) + 0.5f) / static_cast<float>(d[0]),
							(static_cast<float>(y) + 0.5f) / static_cast<float>(d[1]));

//			scene::shape::Sample sample;
//			shape.sample(part, transformation, float3::identity(),
//						 uv, area, sample);

			const float3 wi = unclipped_canopy_mapping(transformation, uv);

			/*if (0.f == sample.pdf) {
				cache->at(x, y) = packed_float3::identity;
			} else*/ {
				const float3 radiance = model_.evaluate_sky(/*sample.*/wi);
				cache->at(x, y) = packed_float3(radiance);
			}
		}
	}

	auto cache_texture = std::make_shared<image::texture::Float3>(cache);
	emission_map_ = Texture_adapter(cache_texture);

//	std::ofstream stream("sky.png", std::ios::binary);
//	if (stream) {
//		image::encoding::png::Writer writer(d);
//		writer.write(stream, *cache, pool);
//	}

	if (importance_sampling) {
		float3 average_radiance = float3::identity();

		float total_weight = 0.f;

		std::vector<float> luminance(d[0] * d[1]);

		for (int32_t y = 0, l = 0; y < d[1]; ++y) {
			for (int32_t x = 0; x < d[0]; ++x, ++l) {
				const float3 radiance = float3(cache->at(x, y));

				const float2 uv((static_cast<float>(x) + 0.5f) / static_cast<float>(d[0]),
								(static_cast<float>(y) + 0.5f) / static_cast<float>(d[1]));

				const float weight = shape.uv_weight(uv);

				average_radiance += weight * radiance;

				total_weight += weight;

				luminance[l] = weight * spectrum::luminance(radiance);
			}
		}

		average_emission_ = average_radiance / total_weight;

		total_weight_ = total_weight;

		distribution_.init(luminance.data(), d);
	} else {
		// This controls how often the sky will be sampled,
		// Zenith sample cause less variance in one test (favoring the sun)...
		// average_emission_ = cache_texture->average_3();
		average_emission_ = model_.evaluate_sky(model_.zenith());
	}
}

float Sky_baked_material::ior() const {
	return 1.5f;
}

size_t Sky_baked_material::num_bytes() const {
	return sizeof(*this) + emission_map_.texture()->image()->num_bytes()
		 + distribution_.num_bytes();
}

float3 Sky_baked_material::unclipped_canopy_mapping(const Transformation& transformation,
													float2 uv) {
	const float2 disk(2.f * uv[0] - 1.f, 2.f * uv[1] - 1.f);

	const float3 dir = math::disk_to_hemisphere_equidistant(disk);

	return math::transform_vector(dir, transformation.rotation);
}

}
