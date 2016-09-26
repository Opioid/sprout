#include "sky_material.hpp"
#include "sky_model.hpp"
#include "core/image/typed_image.inl"
#include "core/scene/prop.hpp"
#include "core/scene/scene_renderstate.hpp"
#include "core/scene/scene_worker.hpp"
#include "core/scene/shape/shape_sample.hpp"
#include "core/scene/shape/shape.hpp"
#include "core/scene/material/material_sample.inl"
#include "core/scene/material/material_sample_cache.inl"
#include "core/scene/shape/geometry/hitpoint.inl"
#include "base/math/vector.inl"

#include "core/image/encoding/png/png_writer.hpp"
#include <fstream>
#include <iostream>

namespace procedural { namespace sky {

Material::Material(scene::material::Generic_sample_cache<scene::material::light::Sample>& cache,
				   Model& model) :
	scene::material::Typed_material<
		scene::material::Generic_sample_cache<scene::material::light::Sample>>(
			cache, scene::material::Sampler_settings(), false),
	model_(model) {}

bool Material::has_emission_map() const {
	return false;
}

Sky_material::Sky_material(
		scene::material::Generic_sample_cache<scene::material::light::Sample>& cache,
		Model& model) : Material(cache, model) {}

const scene::material::Sample& Sky_material::sample(float3_p wo, const scene::Renderstate& rs,
													const scene::Worker& worker,
													Sampler_filter /*filter*/) {
	auto& sample = cache_.get(worker.id());

	sample.set_basis(rs.geo_n, wo);

	sample.layer_.set_basis(rs.t, rs.b, rs.n);

	sample.layer_.set(model_.evaluate_sky(-wo));

	return sample;
}

float3 Sky_material::sample_radiance(float3_p wi, float2 /*uv*/,
									 float /*area*/, float /*time*/,
									 const scene::Worker& /*worker*/,
									 Sampler_filter /*filter*/) const {
	return model_.evaluate_sky(wi);
}

float3 Sky_material::average_radiance(float /*area*/) const {
	return model_.evaluate_sky(model_.zenith());
}

void Sky_material::prepare_sampling(const scene::shape::Shape& shape, uint32_t part,
									const Transformation& transformation,
									float area, thread::Pool& pool) {
	if (!model_.init()) {
		return;
	}

	std::cout << "Let's bake sky stuff" << std::endl;

	int2 d(256, 256);

	image::Image::Description description(image::Image::Type::Float_3, d);
	image::Image_float_3 cache(description);

	for (int y = 0; y < d.y; ++y) {
		for (int x = 0; x < d.x; ++x) {
			float2 uv((static_cast<float>(x) + 0.5f) / static_cast<float>(d.x),
					  (static_cast<float>(y) + 0.5f) / static_cast<float>(d.y));

			scene::shape::Sample sample;
			shape.sample(part, transformation, math::float3_identity,
						 uv, area, sample);

			if (0.f == sample.pdf) {
				cache.at(x, y) = math::packed_float3::identity;
			} else {
				float3 radiance = model_.evaluate_sky(sample.wi);
				cache.at(x, y) = math::packed_float3(0.05f * radiance);
			}

		}
	}

	std::ofstream stream("sky.png", std::ios::binary);
	if (stream) {
		image::encoding::png::Writer writer(d);
		writer.write(stream, cache, pool);
	}
}

void Sky_material::prepare_sampling() {
	model_.init();
}

size_t Sky_material::num_bytes() const {
	return sizeof(*this);
}

Sun_material::Sun_material(
		scene::material::Generic_sample_cache<scene::material::light::Sample>& cache,
		Model& model) : Material(cache, model) {}

const scene::material::Sample& Sun_material::sample(float3_p wo, const scene::Renderstate& rs,
													const scene::Worker& worker,
													Sampler_filter /*filter*/) {
	auto& sample = cache_.get(worker.id());

	sample.set_basis(rs.geo_n, wo);

	sample.layer_.set_basis(rs.t, rs.b, rs.n);

	sample.layer_.set(model_.evaluate_sky_and_sun(-wo));

	return sample;
}

float3 Sun_material::sample_radiance(float3_p wi, float2 /*uv*/,
									 float /*area*/, float /*time*/,
									 const scene::Worker& /*worker*/,
									 Sampler_filter /*filter*/) const {
	return model_.evaluate_sky_and_sun(wi);
}

float3 Sun_material::average_radiance(float /*area*/) const {
	return model_.evaluate_sky_and_sun(-model_.sun_direction());
}

void Sun_material::prepare_sampling(const scene::shape::Shape& /*shape*/, uint32_t /*part*/,
									const Transformation& /*transformation*/,
									float /*area*/, thread::Pool& /*pool*/) {
	model_.init();
}

void Sun_material::prepare_sampling() {
	model_.init();
}

size_t Sun_material::num_bytes() const {
	return sizeof(*this);
}

}}
