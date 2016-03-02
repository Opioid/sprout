#pragma once

#include "triangle_primitive_yf.hpp"

namespace scene { namespace shape { namespace triangle {

inline bool same_sign(float a, float b) {
	return a * b >= 0.f;
}

Triangle_YF::Triangle_YF(const Vertex& a, const Vertex& b, const Vertex& c, uint32_t material_index) :
	a(a), b(b), c(c), material_index(material_index) {
	math::float3 e1 = b.p - a.p;
	math::float3 e2 = c.p - a.p;

	n = math::cross(e1, e2);
	d = math::dot(n, a.p);

	n1 = math::cross(e2, n) / math::squared_length(n);
	d1 = -math::dot(n1, a.p);

	n2 = (math::cross(n, e1) / math::squared_length(n));
	d2 = -math::dot(n2, a.p);
}

inline bool Triangle_YF::intersect(math::Oray& ray, math::float2& uv) const {
	float det = math::dot(ray.direction, n);

	float t = d - math::dot(ray.origin, n);

	if (!same_sign(t, det * ray.max_t - t)) {
		return false;
	}

	math::float3 p = det * ray.origin + t * ray.direction;

	float u = math::dot(p, n1) + det * d1;

	if (!same_sign(u, det - u)) {
		return false;
	}

	float v = math::dot(p, n2) + det * d2;

	if (!same_sign(v, det - u - v)) {
		return false;
	}

	float inv_det = 1.f / det;

	float hit_t = t * inv_det;

	if (hit_t > ray.min_t) {
		ray.max_t = hit_t;
		uv.x = u * inv_det;
		uv.y = v * inv_det;

		return true;
	}

	return false;
}

inline bool Triangle_YF::intersect_p(const math::Oray& ray) const {
	float det = math::dot(ray.direction, n);

	float t = d - math::dot(ray.origin, n);

	if (!same_sign(t, det * ray.max_t - t)) {
		return false;
	}

	math::float3 p = det * ray.origin + t * ray.direction;

	float u = (math::dot(p, n1) + det * d1);

	if (!same_sign(u, det - u)) {
		return false;
	}

	float v = (math::dot(p, n2) + det * d2);

	if (!same_sign(v, det - u - v)) {
		return false;
	}

	float hit_t = t / det;

	if (hit_t > ray.min_t) {
		return true;
	}

	return false;
}

inline void Triangle_YF::interpolate(math::float2 uv, math::float3& p, math::float3& n, math::float2& tc) const {
	float w = 1.f - uv.x - uv.y;

	p  = w * a.p + uv.x * b.p + uv.y * c.p;
	n  = math::normalized(w * a.n + uv.x * b.n + uv.y * c.n);
	tc = w * a.uv + uv.x * b.uv + uv.y * c.uv;
}

inline void Triangle_YF::interpolate_data(math::float2 uv, math::float3& n, math::float3& t, math::float2& tc) const {
	float w = 1.f - uv.x - uv.y;

	n  = math::normalized(w * a.n + uv.x * b.n + uv.y * c.n);
	t  = math::normalized(w * a.t + uv.x * b.t + uv.y * c.t);
	tc = w * a.uv + uv.x * b.uv + uv.y * c.uv;
}

inline math::float2 Triangle_YF::interpolate_uv(math::float2 uv) const {
	float w = 1.f - uv.x - uv.y;

	return w * a.uv + uv.x * b.uv + uv.y * c.uv;
}

inline math::float3 Triangle_YF::normal() const {
	return math::normalized(n);
}

inline float Triangle_YF::area(const math::float3& scale) const {
	math::float3 sa = scale * a.p;
	math::float3 sb = scale * b.p;
	math::float3 sc = scale * c.p;
	return 0.5f * math::length(math::cross(sb - sa, sc - sa));
}

}}}

