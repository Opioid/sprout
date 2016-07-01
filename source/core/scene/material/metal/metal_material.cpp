#include "metal_material.hpp"
#include "metal_sample.hpp"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.hpp"
#include "image/texture/sampler/sampler_2d.hpp"
#include "scene/material/material_sample.inl"
#include "scene/material/material_sample_cache.inl"
#include "scene/shape/geometry/hitpoint.inl"
#include "base/math/vector.inl"

// http://www.3delight.com/en/modules/forum/viewtopic.php?t=4205
// Table of wavelength-dependent values for ior and absorption (k)
// Metal                 index (n)              extinction (k)
// ---------  ----------------------------   ------------------------
// Aluminium  (1.50694, 0.926041, 0.68251)   (7.6307, 6.3849, 5.6230)
// Chromium   (3.11847, 3.02492, 2.44207)    (3.3190, 3.3322, 3.2034)
// Cobalt     (2.86991, 2.19091, 1.80915)    (4.3419, 3.8804, 3.4920)
// Copper     (0.23268, 1.075714, 1.15753)   (3.5315, 2.5945, 2.4473)
// Gold       (0.18267, 0.49447, 1.3761)     (3.1178, 2.3515, 1.8324)
// Iridium    (2.54731, 2.13062, 1.86627)    (4.6443, 4.2061, 3.7653)
// Lithium    (0.22336, 0.20973, 0.23001)    (2.9727, 2.3544, 1.9843)
// Molybdenu  (3.72055, 3.70022, 3.15441)    (3.5606, 3.6863, 3.5371)
// Nickel     (1.99412, 1.73204, 1.64542)    (3.7682, 3.1316, 2.7371)
// Osmium     (3.85401, 4.95607, 5.31627)    (1.6997, 1.8716, 3.2281)
// Palladium  (1.78071, 1.59124, 1.44152)    (4.3345, 3.7268, 3.3428)
// Platinum   (2.35423, 2.06875, 1.88085)    (4.1988, 3.6120, 3.2271)
// Rhodium    (2.17407, 1.92204, 1.81356)    (5.6790, 4.8682, 4.5063)
// Silver     (0.13708, 0.12945, 0.14075)    (4.0625, 3.1692, 2.6034)
// Tantalum   (1.65178, 2.61195, 2.83282)    (2.1475, 1.8786, 2.0726)
// Titanium   (2.18385, 1.8364, 1.72103)     (2.9550, 2.5188, 2.3034)
// Tungsten   (3.66845, 3.48243, 3.31552)    (2.9274, 2.7200, 2.5721)
// Vanadium   (3.51454, 3.66353, 3.23655)    (2.9684, 3.0917, 3.3679)

namespace scene { namespace material { namespace metal {

Material_isotropic::Material_isotropic(Generic_sample_cache<Sample_isotropic>& cache,
									   const Sampler_settings& sampler_settings, bool two_sided) :
	material::Typed_material<Generic_sample_cache<Sample_isotropic>>(cache,
																	 sampler_settings,
																	 two_sided) {}

const material::Sample& Material_isotropic::sample(float3_p wo, const Renderstate& rs,
												   const Worker& worker,
												   Sampler_settings::Filter filter) {
	auto& sample = cache_.get(worker.id());

	sample.set_basis(rs.geo_n, wo);

	if (normal_map_.is_valid()) {
		auto& sampler = worker.sampler(sampler_key_, filter);

		float3 nm = normal_map_.sample_3(sampler, rs.uv);
		float3 n  = math::normalized(rs.tangent_to_world(nm));
		sample.layer_.set_basis(rs.t, rs.b, n);
	} else {
		sample.layer_.set_basis(rs.t, rs.b, rs.n);
	}

	sample.layer_.set(ior_, absorption_, roughness_);

	return sample;
}

void Material_isotropic::set_normal_map(const Adapter_2D& normal_map) {
	normal_map_ = normal_map;
}

void Material_isotropic::set_ior(float3_p ior) {
	ior_ = ior;
}

void Material_isotropic::set_absorption(float3_p absorption) {
	absorption_ = absorption;
}

void Material_isotropic::set_roughness(float roughness) {
	roughness_ = roughness;
}

Material_anisotropic::Material_anisotropic(Generic_sample_cache<Sample_anisotropic>& cache,
										   const Sampler_settings& sampler_settings,
										   bool two_sided) :
	material::Typed_material<Generic_sample_cache<Sample_anisotropic>>(cache,
																	   sampler_settings,
																	   two_sided) {}

const material::Sample& Material_anisotropic::sample(float3_p wo, const Renderstate& rs,
													 const Worker& worker,
													 Sampler_settings::Filter filter) {
	auto& sample = cache_.get(worker.id());

	auto& sampler = worker.sampler(sampler_key_, filter);

	sample.set_basis(rs.geo_n, wo);

	if (normal_map_.is_valid()) {
		float3 nm = normal_map_.sample_3(sampler, rs.uv);
		float3 n  = math::normalized(rs.tangent_to_world(nm));

		sample.layer_.set_basis(rs.t, rs.b, n);
	} else if (direction_map_.is_valid()) {
		float2 tm = direction_map_.sample_2(sampler, rs.uv);
		float3 t  = math::normalized(rs.tangent_to_world(tm));
		float3 b  = math::cross(rs.n, t);

		sample.layer_.set_basis(t, b, rs.n);
	} else {
		sample.layer_.set_basis(rs.t, rs.b, rs.n);
	}

	sample.layer_.set(ior_, absorption_, roughness_);

	return sample;
}

void Material_anisotropic::set_normal_map(const Adapter_2D& normal_map) {
	normal_map_ = normal_map;
}

void Material_anisotropic::set_direction_map(const Adapter_2D& direction_map) {
	direction_map_ = direction_map;
}

void Material_anisotropic::set_ior(float3_p ior) {
	ior_ = ior;
}

void Material_anisotropic::set_absorption(float3_p absorption) {
	absorption_ = absorption;
}

void Material_anisotropic::set_roughness(float2 roughness) {
	roughness_ = roughness;
}

}}}
