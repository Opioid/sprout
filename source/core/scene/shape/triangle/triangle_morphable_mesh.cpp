#include "triangle_morphable_mesh.hpp"
#include "triangle_intersection.hpp"
#include "triangle_morph_target_collection.hpp"
#include "triangle_primitive_mt.hpp"
#include "bvh/triangle_bvh_builder_sah2.inl"
#include "bvh/triangle_bvh_indexed_data.inl"
//#include "bvh/triangle_bvh_data_interleaved.inl"
#include "bvh/triangle_bvh_tree.inl"
#include "scene/scene_ray.inl"
#include "scene/entity/composed_transformation.hpp"
#include "scene/shape/shape_intersection.hpp"
#include "scene/shape/shape_sample.hpp"
#include "sampler/sampler.hpp"
#include "base/math/vector3.inl"
#include "base/math/matrix3x3.inl"
#include "base/math/distribution/distribution_1d.inl"

namespace scene { namespace shape { namespace triangle {

Morphable_mesh::Morphable_mesh(std::shared_ptr<Morph_target_collection> collection,
							   uint32_t num_parts) :
	collection_(collection) {
	tree_.allocate_parts(num_parts);
	vertices_.resize(collection_->vertices(0).size());
}

void Morphable_mesh::init() {
	aabb_ = tree_.aabb();
}

uint32_t Morphable_mesh::num_parts() const {
	return tree_.num_parts();
}

bool Morphable_mesh::intersect(const Transformation& transformation, Ray& ray,
							   Node_stack& node_stack, shape::Intersection& intersection) const {
	math::Ray tray;
	tray.origin = math::transform_point(ray.origin, transformation.world_to_object);
	tray.set_direction(math::transform_vector(ray.direction, transformation.world_to_object));
	tray.min_t = ray.min_t;
	tray.max_t = ray.max_t;

	Intersection pi;
	if (tree_.intersect(tray, node_stack, pi)) {
		ray.max_t = tray.max_t;

		float epsilon = 3e-3f * tray.max_t;

		float3 p_w = ray.point(tray.max_t);

		float3 n;
		float3 t;
		float2 uv;
	//	tree_.interpolate_triangle_data(pi.index, pi.uv, n, t, uv);
		tree_.interpolate_triangle_data(pi.u, pi.v, pi.index, n, t, uv);

		float3	 geo_n			= tree_.triangle_normal(pi.index);
		float	 bitangent_sign = tree_.triangle_bitangent_sign(pi.index);
		uint32_t material_index = tree_.triangle_material_index(pi.index);

		float3 geo_n_w = math::transform_vector(geo_n, transformation.rotation);
		float3 n_w	   = math::transform_vector(n, transformation.rotation);
		float3 t_w	   = math::transform_vector(t, transformation.rotation);
		float3 b_w	   = bitangent_sign * math::cross(n_w, t_w);

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

bool Morphable_mesh::intersect_p(const Transformation& transformation,
								 const Ray& ray, Node_stack& node_stack) const {
	math::Ray tray;
	tray.origin = math::transform_point(ray.origin, transformation.world_to_object);
	tray.set_direction(math::transform_vector(ray.direction, transformation.world_to_object));
	tray.min_t = ray.min_t;
	tray.max_t = ray.max_t;

	return tree_.intersect_p(tray, node_stack);
}

float Morphable_mesh::opacity(const Transformation& transformation, const Ray& ray,
							  const material::Materials& materials,
							  Worker& worker, Sampler_filter filter) const {
	math::Ray tray;
	tray.origin = math::transform_point(ray.origin, transformation.world_to_object);
	tray.set_direction(math::transform_vector(ray.direction, transformation.world_to_object));
	tray.min_t = ray.min_t;
	tray.max_t = ray.max_t;

	return tree_.opacity(tray, ray.time, materials, worker, filter);
}

float3 Morphable_mesh::thin_absorption(const Transformation& transformation, const Ray& ray,
									   const material::Materials& materials,
									   Worker& worker, Sampler_filter filter) const {
	math::Ray tray;
	tray.origin = math::transform_point(ray.origin, transformation.world_to_object);
	tray.set_direction(math::transform_vector(ray.direction, transformation.world_to_object));
	tray.min_t = ray.min_t;
	tray.max_t = ray.max_t;

	return tree_.absorption(tray, ray.time, materials, worker, filter);
}

void Morphable_mesh::sample(uint32_t /*part*/, const Transformation& /*transformation*/,
							const float3& /*p*/, const float3& /*n*/, float /*area*/, bool /*two_sided*/,
							sampler::Sampler& /*sampler*/, uint32_t /*sampler_dimension*/,
							Node_stack& /*node_stack*/, Sample& /*sample*/) const {}

void Morphable_mesh::sample(uint32_t /*part*/, const Transformation& /*transformation*/,
							const float3& /*p*/, float /*area*/, bool /*two_sided*/,
							sampler::Sampler& /*sampler*/, uint32_t /*sampler_dimension*/,
							Node_stack& /*node_stack*/, Sample& /*sample*/) const {}

float Morphable_mesh::pdf(uint32_t /*part*/, const Transformation& /*transformation*/,
						  const float3& /*p*/, const float3& /*wi*/, float /*offset*/,
						  float /*area*/, bool /*two_sided*/, bool /*total_sphere*/,
						  shape::Node_stack& /*node_stack*/) const {
	return 1.f;
}

void Morphable_mesh::sample(uint32_t /*part*/, const Transformation& /*transformation*/,
							const float3& /*p*/, float2 /*uv*/, float /*area*/, bool /*two_sided*/,
							Sample& /*sample*/) const {}

float Morphable_mesh::pdf_uv(uint32_t /*part*/, const Transformation& /*transformation*/,
							 const float3& /*p*/, const float3& /*wi*/,
							 float /*area*/, bool /*two_sided*/,
							 float2& /*uv*/) const {
	return 1.f;
}

float Morphable_mesh::pdf_uv(const float3& p, const float3& wi,
							 const shape::Intersection& intersection,
							 const Transformation& transformation,
							 float hit_t, float area, bool two_sided) const {
	return 1.f;
}

float Morphable_mesh::uv_weight(float2 /*uv*/) const {
	return 1.f;
}

float Morphable_mesh::area(uint32_t /*part*/, const float3& /*scale*/) const {
	return 1.f;
}

bool Morphable_mesh::is_complex() const {
	return true;
}

bool Morphable_mesh::is_analytical() const {
	return false;
}

void Morphable_mesh::prepare_sampling(uint32_t /*part*/) {}

Morphable_shape* Morphable_mesh::morphable_shape() {
	return this;
}

void Morphable_mesh::morph(uint32_t a, uint32_t b, float weight, thread::Pool& pool) {
	collection_->morph(a, b, weight, pool, vertices_);

//	bvh::Builder_SUH builder;
//	builder.build(tree_, collection_->triangles(), vertices_, 0, 8);

	bvh::Builder_SAH2 builder(16, 64);
	builder.build(tree_, collection_->triangles(), vertices_, 4, pool);

	init();
}

size_t Morphable_mesh::num_bytes() const {
	return sizeof(*this);
}

}}}


