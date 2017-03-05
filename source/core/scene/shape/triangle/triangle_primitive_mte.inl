#pragma once

#include "triangle_primitive_mte.hpp"

namespace scene { namespace shape { namespace triangle {

inline Triangle_MTE::Triangle_MTE(const shape::Vertex& a,
								  const shape::Vertex& b,
								  const shape::Vertex& c,
								  float bitangent_sign,
								  uint32_t material_index) :
	ap(a.p), e1(b.p - a.p), e2(c.p - a.p),
	an(a.n), bn(b.n), cn(c.n),
	at(a.t), bt(b.t), ct(c.t),
	auv(a.uv), buv(b.uv), cuv(c.uv),
	bitangent_sign(bitangent_sign), material_index(material_index) {}

inline bool Triangle_MTE::intersect(math::Ray& ray, float2& uv) const {
	float3 pvec = math::cross(ray.direction, e2);

	float det = math::dot(e1, pvec);
	float inv_det = 1.f / det;

	float3 tvec = ray.origin - ap;
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
		uv[0] = u;
		uv[1] = v;
		return true;
	}

	return false;
}

inline bool Triangle_MTE::intersect_p(const math::Ray& ray) const {
	float3 pvec = math::cross(ray.direction, e2);

	float det = math::dot(e1, pvec);
	float inv_det = 1.f / det;

	float3 tvec = ray.origin - ap;
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

inline void Triangle_MTE::interpolate(float2 uv, float3& p, float2& tc) const {
	float w = 1.f - uv[0] - uv[1];

	p  = w * ap + uv[0] * (ap + e1)  + uv[1] * (ap + e2);
	tc = w * auv + uv[0] * buv + uv[1] * cuv;
}

inline void Triangle_MTE::interpolate_data(float2 uv,
										   float3& n, float3& t, float2& tc) const {
	float w = 1.f - uv[0] - uv[1];

	n  = math::normalized(w * an + uv[0] * bn + uv[1] * cn);
	t  = math::normalized(w * at + uv[0] * bt + uv[1] * ct);
	tc = w * auv + uv[0] * buv + uv[1] * cuv;
}

inline float2 Triangle_MTE::interpolate_uv(float2 uv) const {
	float w = 1.f - uv[0] - uv[1];

	return w * auv + uv[0] * buv + uv[1] * cuv;
}

inline float3 Triangle_MTE::normal() const {
	return math::normalized(math::cross(e1, e2));
}

inline float Triangle_MTE::area() const {
	return 0.5f * math::length(math::cross(e1, e2));
}

inline float Triangle_MTE::area(float3_p scale) const {
	return 0.5f * math::length(math::cross(scale * e1, scale * e2));
}

inline Intersection_triangle_MTE::Intersection_triangle_MTE(const shape::Vertex& a,
															const shape::Vertex& b,
															const shape::Vertex& c) :
	ap(a.p),
	e1(b.p - a.p),
	e2(c.p - a.p) {}

inline bool Intersection_triangle_MTE::intersect(math::Ray& ray, float2& uv) const {
	float3 pvec = math::cross(ray.direction, e2);

	float det = math::dot(e1, pvec);
	float inv_det = 1.f / det;

	float3 tvec = ray.origin - ap;
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
		uv[0] = u;
		uv[1] = v;
		return true;
	}

	return false;
}

inline bool Intersection_triangle_MTE::intersect_p(const math::Ray& ray) const {
	float3 pvec = math::cross(ray.direction, e2);

	float det = math::dot(e1, pvec);
	float inv_det = 1.f / det;

	float3 tvec = ray.origin - ap;
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

inline void Intersection_triangle_MTE::interpolate(float2 uv, float3& p) const {
	float w = 1.f - uv[0] - uv[1];

	p  = w * ap + uv[0] * (ap + e1)  + uv[1] * (ap + e2);
}

inline float3 Intersection_triangle_MTE::normal() const {
	return math::normalized(math::cross(e1, e2));
}

inline float Intersection_triangle_MTE::area() const {
	return 0.5f * math::length(math::cross(e1, e2));
}

inline float Intersection_triangle_MTE::area(float3_p scale) const {
	return 0.5f * math::length(math::cross(scale * e1, scale * e2));
}

inline Shading_triangle_MTE::Shading_triangle_MTE(const shape::Vertex& a,
												  const shape::Vertex& b,
												  const shape::Vertex& c,
												  uint32_t material_index) :
	an(a.n), bn(b.n), cn(c.n),
	at(a.t), bt(b.t), ct(c.t),
	auv(a.uv), buv(b.uv), cuv(c.uv),
	bitangent_sign(a.bitangent_sign),
	material_index(material_index) {}

inline void Shading_triangle_MTE::interpolate_data(float2 uv,
												   float3& n, float3& t, float2& tc) const {
	float w = 1.f - uv[0] - uv[1];

	n  = math::normalized(w * an + uv[0] * bn + uv[1] * cn);
	t  = math::normalized(w * at + uv[0] * bt + uv[1] * ct);
	tc = w * auv + uv[0] * buv + uv[1] * cuv;
}

inline float2 Shading_triangle_MTE::interpolate_uv(float2 uv) const {
	float w = 1.f - uv[0] - uv[1];

	return w * auv + uv[0] * buv + uv[1] * cuv;
}

}}}
