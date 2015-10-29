#pragma once

#include "triangle_primitive_mt.hpp"

namespace scene { namespace shape { namespace triangle {

inline Triangle_MT::Vertex::Vertex(const shape::Vertex& v) : p(v.p), n(v.n), t(v.t), uv(v.uv) {}

inline bool Triangle_MT::intersect(math::Oray& ray, math::float2& uv) const {
	math::float3 e1 = b.p - a.p;
	math::float3 e2 = c.p - a.p;

	math::float3 pvec = math::cross(ray.direction, e2);

	float det = math::dot(e1, pvec);
	float inv_det = 1.f / det;

	math::float3 tvec = ray.origin - a.p;
	float u = math::dot(tvec, pvec) * inv_det;

	if (u < 0.f || u > 1.f) {
		return false;
	}

	math::float3 qvec = math::cross(tvec, e1);
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
	math::float3 e1 = b.p - a.p;
	math::float3 e2 = c.p - a.p;

	math::float3 pvec = math::cross(ray.direction, e2);

	float det = math::dot(e1, pvec);
	float inv_det = 1.f / det;

	math::float3 tvec = ray.origin - a.p;
	float u = math::dot(tvec, pvec) * inv_det;

	if (u < 0.f || u > 1.f) {
		return false;
	}

	math::float3 qvec = math::cross(tvec, e1);
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

inline void Triangle_MT::interpolate(math::float2 uv, math::float3& p, math::float3& n, math::float2& tc) const {
	float w = 1.f - uv.x - uv.y;

	p  = w * a.p + uv.x * b.p + uv.y * c.p;
	n  = math::normalized(w * a.n + uv.x * b.n + uv.y * c.n);
	tc = w * a.uv + uv.x * b.uv + uv.y * c.uv;
}

inline void Triangle_MT::interpolate(math::float2 uv, math::float3& p, math::float2& tc) const {
	float w = 1.f - uv.x - uv.y;

	p  = w * a.p + uv.x * b.p + uv.y * c.p;
	tc = w * a.uv + uv.x * b.uv + uv.y * c.uv;
}

inline void Triangle_MT::interpolate_data(math::float2 uv,
										  math::float3& n, math::float3& t, math::float2& tc) const {
	float w = 1.f - uv.x - uv.y;

	n  = math::normalized(w * a.n + uv.x * b.n + uv.y * c.n);
	t  = math::normalized(w * a.t + uv.x * b.t + uv.y * c.t);
	tc = w * a.uv + uv.x * b.uv + uv.y * c.uv;
}

inline math::float2 Triangle_MT::interpolate_uv(math::float2 uv) const {
	float w = 1.f - uv.x - uv.y;

	return w * a.uv + uv.x * b.uv + uv.y * c.uv;
}

inline math::float3 Triangle_MT::normal() const {
	math::float3 e1 = b.p - a.p;
	math::float3 e2 = c.p - a.p;

	return math::normalized(math::cross(e1, e2));
}

inline float Triangle_MT::area(const math::float3& scale) const {
	math::float3 sa = scale * a.p;
	math::float3 sb = scale * b.p;
	math::float3 sc = scale * c.p;
	return 0.5f * math::length(math::cross(sb - sa, sc - sa));
}

}}}
