#include "triangle_mesh.hpp"
#include "triangle_intersection.hpp"
#include "bvh/triangle_bvh_tree.inl"
#include "bvh/triangle_bvh_data.inl"
#include "bvh/triangle_bvh_data_interleaved.inl"
#include "scene/entity/composed_transformation.hpp"
#include "scene/shape/shape_sample.hpp"
#include "scene/shape/geometry/shape_intersection.hpp"
#include "sampler/sampler.hpp"
#include "base/math/vector.inl"
#include "base/math/ray.inl"
#include "base/math/matrix.inl"
#include "base/math/distribution/distribution_1d.inl"
#include "base/math/simd/simd_vector.inl"
#include "base/math/simd/simd_matrix.inl"

namespace scene { namespace shape { namespace triangle {

Mesh::Tree& Mesh::tree() {
	return tree_;
}

void Mesh::init() {
	aabb_ = tree_.aabb();

	distributions_.resize(tree_.num_parts());
}

uint32_t Mesh::num_parts() const {
	return tree_.num_parts();
}

bool Mesh::intersect(const entity::Composed_transformation& transformation, math::Oray& ray,
					 Node_stack& node_stack, shape::Intersection& intersection) const {
	math::Oray tray;
	tray.origin = math::transform_point(ray.origin, transformation.world_to_object);
	tray.set_direction(math::transform_vector(ray.direction, transformation.world_to_object));
	tray.min_t = ray.min_t;
	tray.max_t = ray.max_t;

	Intersection pi;
	if (tree_.intersect(tray, node_stack, pi)) {
		ray.max_t = tray.max_t;

		float epsilon = 3e-3f * tray.max_t;

		math::float3 p_w = ray.point(tray.max_t);

		math::float3 n;
		math::float3 t;
		math::float2 uv;
		tree_.interpolate_triangle_data(pi.index, pi.uv, n, t, uv);

		math::float3 geo_n = tree_.triangle_normal(pi.index);

		float bitangent_sign = tree_.triangle_bitangent_sign(pi.index);
		uint32_t material_index = tree_.triangle_material_index(pi.index);

		math::float3 geo_n_w = math::transform_vector(geo_n, transformation.rotation);
		math::float3 n_w	   = math::transform_vector(n, transformation.rotation);
		math::float3 t_w	   = math::transform_vector(t, transformation.rotation);
		math::float3 b_w	   = bitangent_sign * math::cross(n_w, t_w);



/*
		math::simd::Matrix rotation = math::simd::load_float4x4(transformation.rotation);

		math::simd::Vector simd_t = math::simd::load_float3(t);
		math::simd::Vector t_w = math::simd::transform_vector(rotation, simd_t);
		math::simd::store_float3(intersection.t, t_w);

		math::simd::Vector simd_n = math::simd::load_float3(n);
		math::simd::Vector n_w = math::simd::transform_vector(rotation, simd_n);
		math::simd::store_float3(intersection.n, n_w);

		math::simd::Vector simd_geo_n = math::simd::load_float3(geo_n);
		math::simd::Vector geo_n_w = math::simd::transform_vector(rotation, simd_geo_n);
		math::simd::store_float3(intersection.geo_n, geo_n_w);

		math::simd::Vector b_w = math::simd::cross3(n_w, t_w);
		math::float3 ub_w;
		math::simd::store_float3(ub_w, b_w);
		intersection.b = bitangent_sign * ub_w;
*/



		intersection.p = p_w;
		intersection.t = t_w;
		intersection.b = b_w;
		intersection.n = n_w;
		intersection.geo_n = geo_n_w;
		intersection.uv = uv;
		intersection.epsilon = epsilon;
		intersection.part = material_index;

		return true;
	}

	return false;
}

bool Mesh::intersect_p(const entity::Composed_transformation& transformation, const math::Oray& ray,
					   Node_stack& node_stack) const {
	math::Oray tray;
	tray.origin = math::transform_point(ray.origin, transformation.world_to_object);
	tray.set_direction(math::transform_vector(ray.direction, transformation.world_to_object));
	tray.min_t = ray.min_t;
	tray.max_t = ray.max_t;

	return tree_.intersect_p(tray, node_stack);
}

float Mesh::opacity(const entity::Composed_transformation& transformation, const math::Oray& ray,
					float time, Node_stack& node_stack, const material::Materials& materials,
					const image::texture::sampler::Sampler_2D& sampler) const {
	math::Oray tray;
	tray.origin = math::transform_point(ray.origin, transformation.world_to_object);
	tray.set_direction(math::transform_vector(ray.direction, transformation.world_to_object));
	tray.min_t = ray.min_t;
	tray.max_t = ray.max_t;

	return tree_.opacity(tray, time, node_stack, materials, sampler);
}

void Mesh::sample(uint32_t part, const entity::Composed_transformation& transformation, float area,
				  const math::float3& p, const math::float3& /*n*/, bool two_sided,
				  sampler::Sampler& sampler, Node_stack& node_stack, Sample& sample) const {
	Mesh::sample(part, transformation, area, p, two_sided, sampler, node_stack, sample);
}

void Mesh::sample(uint32_t part, const entity::Composed_transformation& transformation, float area,
				  const math::float3& p, bool two_sided,
				  sampler::Sampler& sampler, Node_stack& /*node_stack*/, Sample& sample) const {
	float r = sampler.generate_sample_1D();
	math::float2 r2 = sampler.generate_sample_2D();

	uint32_t index = distributions_[part].sample(r);

	math::float3 sv;
	math::float2 tc;
	tree_.sample(index, r2, sv, tc);
	math::float3 v = math::transform_point(sv, transformation.object_to_world);

	math::float3 sn = tree_.triangle_normal(index);
	math::float3 wn = math::transform_vector(sn, transformation.rotation);

	math::float3 axis = v - p;
	float sl = math::squared_length(axis);
	float d  = std::sqrt(sl);
	math::float3 dir = axis / d;

	float c = math::dot(wn, -dir);

	if (two_sided) {
		c = std::abs(c);
	}

	if (c <= 0.f) {
		sample.pdf = 0.f;
	} else {
		sample.wi = dir;
		sample.uv = tc;
		sample.t = d;
		sample.pdf = sl / (c * area);
	}
}

void Mesh::sample(uint32_t /*part*/, const entity::Composed_transformation& /*transformation*/, float /*area*/,
				  const math::float3& /*p*/, math::float2 /*uv*/, Sample& /*sample*/) const {}

void Mesh::sample(uint32_t /*part*/, const entity::Composed_transformation& /*transformation*/, float /*area*/,
				  const math::float3& /*p*/, const math::float3& /*wi*/, Sample& /*sample*/) const {}

float Mesh::pdf(uint32_t part, const entity::Composed_transformation& transformation, float area,
				const math::float3& p, const math::float3& wi, bool two_sided, bool /*total_sphere*/,
				Node_stack& node_stack) const {
	math::Oray ray;
	ray.origin = math::transform_point(p, transformation.world_to_object);
	ray.set_direction(math::transform_vector(wi, transformation.world_to_object));
	ray.min_t = 0.f;
	ray.max_t = 10000.f;

	Intersection pi;
	if (tree_.intersect(ray, node_stack, pi)) {
		uint32_t shape_part = tree_.triangle_material_index(pi.index);
		if (part != shape_part) {
			return 0.f;
		}

		math::float3 sn = tree_.triangle_normal(pi.index);
		math::float3 wn = math::transform_vector(sn, transformation.rotation);

		float c = math::dot(wn, -wi);

		if (two_sided) {
			c = std::abs(c);
		}

		if (c <= 0.f) {
			return 0.f;
		}

		float sl = ray.max_t * ray.max_t;
		return sl / (c * area);
	}

	return 0.f;
}

float Mesh::area(uint32_t part, const math::float3& scale) const {
	return distributions_[part].distribution.integral() * scale.x * scale.x;
}

bool Mesh::is_complex() const {
	return true;
}

bool Mesh::is_analytical() const {
	return false;
}

void Mesh::prepare_sampling(uint32_t part) {
   if (distributions_[part].empty()) {
	   distributions_[part].init(part, tree_);
   }
}

void Mesh::Distribution::init(uint32_t part, const Tree& tree) {
	uint32_t num_triangles = tree.num_triangles(part);

	std::vector<float> areas(num_triangles);

	triangle_mapping.resize(num_triangles);

	for (uint32_t t = 0, mt = 0, len = tree.num_triangles(); t < len; ++t) {
        if (tree.triangle_material_index(t) == part) {
			areas[mt] = tree.triangle_area(t);
			triangle_mapping[mt] = t;
			++mt;
		}
	}

	distribution.init(areas.data(), num_triangles);
}

bool Mesh::Distribution::empty() const {
	return triangle_mapping.empty();
}

uint32_t Mesh::Distribution::sample(float r) const {
	return triangle_mapping[distribution.sample_discrete(r)];
}

}}}
