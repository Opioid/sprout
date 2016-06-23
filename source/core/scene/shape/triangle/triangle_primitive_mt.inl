#pragma once

#include "triangle_primitive_mt.hpp"

namespace scene { namespace shape { namespace triangle {

inline Triangle_MT::Triangle_MT(const shape::Vertex& a,
								const shape::Vertex& b,
								const shape::Vertex& c,
								uint32_t material_index) :
	a(a), b(b), c(c), bitangent_sign(a.bitangent_sign), material_index(material_index) {}

inline Triangle_MT::Vertex::Vertex(const shape::Vertex& v) : p(v.p), n(v.n), t(v.t), uv(v.uv) {}

inline bool Triangle_MT::intersect(math::Oray& ray, float2& uv) const {
	float3 e1 = b.p - a.p;
	float3 e2 = c.p - a.p;

	float3 pvec = math::cross(ray.direction, e2);

	float det = math::dot(e1, pvec);
	float inv_det = 1.f / det;

	float3 tvec = ray.origin - a.p;
	float u = math::dot(tvec, pvec) * inv_det;

	if (u < 0.f || u > 1.f) {
		return false;
	}

	float3 qvec = math::cross(tvec, e1);
	float v = math::dot(ray.direction, qvec) * inv_det;

	if (v < 0.f || u + v > 1.f) {
		return false;
	}

	float hit_t = math::dot(e2, qvec) * inv_det;

	if (hit_t > ray.min_t && hit_t < ray.max_t) {
		ray.max_t = hit_t;
		uv.x = u;
		uv.y = v;
		return true;
	}

	return false;
}

inline bool Triangle_MT::intersect_p(const math::Oray& ray) const {
	float3 e1 = b.p - a.p;
	float3 e2 = c.p - a.p;

	float3 pvec = math::cross(ray.direction, e2);

	float det = math::dot(e1, pvec);
	float inv_det = 1.f / det;

	float3 tvec = ray.origin - a.p;
	float u = math::dot(tvec, pvec) * inv_det;

	if (u < 0.f || u > 1.f) {
		return false;
	}

	float3 qvec = math::cross(tvec, e1);
	float v = math::dot(ray.direction, qvec) * inv_det;

	if (v < 0.f || u + v > 1.f) {
		return false;
	}

	float hit_t = math::dot(e2, qvec) * inv_det;

	if (hit_t > ray.min_t && hit_t < ray.max_t) {
		return true;
	}

	return false;
}

inline void Triangle_MT::interpolate(float2 uv, float3& p, float3& n,
									 float2& tc) const {
	float w = 1.f - uv.x - uv.y;

	p  = w * a.p + uv.x * b.p + uv.y * c.p;
	n  = math::normalized(w * a.n + uv.x * b.n + uv.y * c.n);
	tc = w * a.uv + uv.x * b.uv + uv.y * c.uv;
}

inline void Triangle_MT::interpolate(float2 uv, float3& p, float2& tc) const {
	float w = 1.f - uv.x - uv.y;

	p  = w * a.p + uv.x * b.p + uv.y * c.p;
	tc = w * a.uv + uv.x * b.uv + uv.y * c.uv;
}

inline void Triangle_MT::interpolate(float2 uv, float3& p) const {
	float w = 1.f - uv.x - uv.y;

	p = w * a.p + uv.x * b.p + uv.y * c.p;
}

inline void Triangle_MT::interpolate_data(float2 uv,
										  float3& n, float3& t,
										  float2& tc) const {
	float w = 1.f - uv.x - uv.y;

	n  = math::normalized(w * a.n + uv.x * b.n + uv.y * c.n);
	t  = math::normalized(w * a.t + uv.x * b.t + uv.y * c.t);
	tc = w * a.uv + uv.x * b.uv + uv.y * c.uv;
}

inline float2 Triangle_MT::interpolate_uv(float2 uv) const {
	float w = 1.f - uv.x - uv.y;

	return w * a.uv + uv.x * b.uv + uv.y * c.uv;
}

inline float3 Triangle_MT::normal() const {
	float3 e1 = b.p - a.p;
	float3 e2 = c.p - a.p;

	return math::normalized(math::cross(e1, e2));
}

inline float Triangle_MT::area() const {
	return 0.5f * math::length(math::cross(b.p - a.p, c.p - a.p));
}

inline float Triangle_MT::area(const float3& scale) const {
	float3 sa = scale * a.p;
	float3 sb = scale * b.p;
	float3 sc = scale * c.p;
	return 0.5f * math::length(math::cross(sb - sa, sc - sa));
}

inline bool intersect(const Intersection_vertex_MT& a,
					  const Intersection_vertex_MT& b,
					  const Intersection_vertex_MT& c,
					  math::Oray& ray, float2& uv) {
	float3 e1 = b.p - a.p;
	float3 e2 = c.p - a.p;

	float3 pvec = math::cross(ray.direction, e2);

	float det = math::dot(e1, pvec);
	float inv_det = 1.f / det;

	float3 tvec = ray.origin - a.p;
	float u = math::dot(tvec, pvec) * inv_det;

	if (u < 0.f || u > 1.f) {
		return false;
	}

	float3 qvec = math::cross(tvec, e1);
	float v = math::dot(ray.direction, qvec) * inv_det;

	if (v < 0.f || u + v > 1.f) {
		return false;
	}

	float hit_t = math::dot(e2, qvec) * inv_det;

	if (hit_t > ray.min_t && hit_t < ray.max_t) {
		ray.max_t = hit_t;
		uv.x = u;
		uv.y = v;
		return true;
	}

	return false;
}

inline bool intersect_p(const Intersection_vertex_MT& a,
						const Intersection_vertex_MT& b,
						const Intersection_vertex_MT& c,
						const math::Oray& ray) {
	float3 e1 = b.p - a.p;
	float3 e2 = c.p - a.p;

	float3 pvec = math::cross(ray.direction, e2);

	float det = math::dot(e1, pvec);
	float inv_det = 1.f / det;

	float3 tvec = ray.origin - a.p;
	float u = math::dot(tvec, pvec) * inv_det;

	if (u < 0.f || u > 1.f) {
		return false;
	}

	float3 qvec = math::cross(tvec, e1);
	float v = math::dot(ray.direction, qvec) * inv_det;

	if (v < 0.f || u + v > 1.f) {
		return false;
	}

	float hit_t = math::dot(e2, qvec) * inv_det;

	if (hit_t > ray.min_t && hit_t < ray.max_t) {
		return true;
	}

	return false;
}

inline void interpolate_p(const Intersection_vertex_MT& a,
						  const Intersection_vertex_MT& b,
						  const Intersection_vertex_MT& c,
						  float2 uv, float3& p) {
	float w = 1.f - uv.x - uv.y;

	p = w * a.p + uv.x * b.p + uv.y * c.p;
}

inline float2 interpolate_uv(const Shading_vertex_MT& a,
								   const Shading_vertex_MT& b,
								   const Shading_vertex_MT& c,
								   float2 uv) {
	float w = 1.f - uv.x - uv.y;

	return w * a.uv + uv.x * b.uv + uv.y * c.uv;
}

inline void interpolate_data(const Shading_vertex_MT& a,
							 const Shading_vertex_MT& b,
							 const Shading_vertex_MT& c,
							 float2 uv,
							 float3& n, float3& t, float2& tc) {
	float w = 1.f - uv.x - uv.y;

	n  = math::normalized(w * a.n + uv.x * b.n + uv.y * c.n);
	t  = math::normalized(w * a.t + uv.x * b.t + uv.y * c.t);
	tc = w * a.uv + uv.x * b.uv + uv.y * c.uv;
}

inline float area(const Intersection_vertex_MT& a,
				  const Intersection_vertex_MT& b,
				  const Intersection_vertex_MT& c) {
	return 0.5f * math::length(math::cross(b.p - a.p, c.p - a.p));
}

inline float area(const Intersection_vertex_MT& a,
				  const Intersection_vertex_MT& b,
				  const Intersection_vertex_MT& c,
				  const float3& scale) {
	float3 sa = scale * a.p;
	float3 sb = scale * b.p;
	float3 sc = scale * c.p;
	return 0.5f * math::length(math::cross(sb - sa, sc - sa));
}

inline float2 interpolate_uv(const Shading_vertex_MTC& a,
								   const Shading_vertex_MTC& b,
								   const Shading_vertex_MTC& c,
								   float2 uv) {
	float w = 1.f - uv.x - uv.y;

	return float2(w * a.n_u.w + uv.x * b.n_u.w + uv.y * c.n_u.w,
						w * a.t_v.w + uv.x * b.t_v.w + uv.y * c.t_v.w);
}

inline void interpolate_data(const Shading_vertex_MTC& a,
							 const Shading_vertex_MTC& b,
							 const Shading_vertex_MTC& c,
							 float2 uv,
							 float3& n, float3& t, float2& tc) {
	float w = 1.f - uv.x - uv.y;

	n  = math::normalized(w * a.n_u.xyz + uv.x * b.n_u.xyz + uv.y * c.n_u.xyz);
	t  = math::normalized(w * a.t_v.xyz + uv.x * b.t_v.xyz + uv.y * c.t_v.xyz);

	tc = float2(w * a.n_u.w + uv.x * b.n_u.w + uv.y * c.n_u.w,
					  w * a.t_v.w + uv.x * b.t_v.w + uv.y * c.t_v.w);
}

}}}
