#pragma once

#include "triangle_primitive.hpp"

namespace scene { namespace shape { namespace triangle {

inline bool Triangle::intersect(math::Oray& ray, math::float2& uv) const {
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

inline bool Triangle::intersect_p(const math::Oray& ray) const {
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

inline void Triangle::interpolate(math::float2 uv, math::float3& p, math::float3& n, math::float2& tc) const {
	float w = 1.f - uv.x - uv.y;

	p  = w * a.p + uv.x * b.p + uv.y * c.p;
	n  = math::normalized(w * a.n + uv.x * b.n + uv.y * c.n);
	tc = w * a.uv + uv.x * b.uv + uv.y * c.uv;
}

inline void Triangle::interpolate_data(math::float2 uv, math::float3& n, math::float3& t, math::float2& tc) const {
	float w = 1.f - uv.x - uv.y;

	n  = math::normalized(w * a.n + uv.x * b.n + uv.y * c.n);
	t  = math::normalized(w * a.t + uv.x * b.t + uv.y * c.t);
	tc = w * a.uv + uv.x * b.uv + uv.y * c.uv;
}

inline math::float2 Triangle::interpolate_uv(math::float2 uv) const {
	float w = 1.f - uv.x - uv.y;

	return w * a.uv + uv.x * b.uv + uv.y * c.uv;
}

inline math::float3 Triangle::normal() const {
	math::float3 e1 = b.p - a.p;
	math::float3 e2 = c.p - a.p;

	return math::normalized(math::cross(e1, e2));
}

inline float Triangle::area(const math::float3& scale) const {
	math::float3 sa = scale * a.p;
	math::float3 sb = scale * b.p;
	math::float3 sc = scale * c.p;
	return 0.5f * math::length(math::cross(sb - sa, sc - sa));
}

inline bool Position_triangle::intersect(math::Oray& ray, math::float2& uv) const {
	math::float3 e1 = b - a;
	math::float3 e2 = c - a;

	math::float3 pvec = math::cross(ray.direction, e2);

	float det = math::dot(e1, pvec);
	float inv_det = 1.f / det;

	math::float3 tvec = ray.origin - a;
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

inline bool Position_triangle::intersect_p(const math::Oray& ray) const {
	math::float3 e1 = b - a;
	math::float3 e2 = c - a;

	math::float3 pvec = math::cross(ray.direction, e2);

	float det = math::dot(e1, pvec);
	float inv_det = 1.f / det;

	math::float3 tvec = ray.origin - a;
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

inline math::float3 Position_triangle::normal() const {
	math::float3 e1 = b - a;
	math::float3 e2 = c - a;

	return math::normalized(math::cross(e1, e2));
}

inline float Position_triangle::area(const math::float3& scale) const {
	math::float3 sa = scale * a;
	math::float3 sb = scale * b;
	math::float3 sc = scale * c;
	return 0.5f * math::length(math::cross(sb - sa, sc - sa));
}

inline void Data_triangle::interpolate_data(math::float2 uv, math::float3& n, math::float3& t, math::float2& tc) const {
	float w = 1.f - uv.x - uv.y;

	n  = math::normalized(w * a.n + uv.x * b.n + uv.y * c.n);
	t  = math::normalized(w * a.t + uv.x * b.t + uv.y * c.t);
	tc = w * a.uv + uv.x * b.uv + uv.y * c.uv;
}

inline math::float2 Data_triangle::interpolate_uv(math::float2 uv) const {
	float w = 1.f - uv.x - uv.y;

	return w * a.uv + uv.x * b.uv + uv.y * c.uv;
}

inline bool intersect(const math::float3& a, const math::float3& b, const math::float3& c, math::Oray& ray, math::float2& uv) {
	math::float3 e1 = b - a;
	math::float3 e2 = c - a;

	math::float3 pvec = math::cross(ray.direction, e2);

	float det = math::dot(e1, pvec);
	float inv_det = 1.f / det;

	math::float3 tvec = ray.origin - a;
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

inline bool intersect_p(const math::float3& a, const math::float3& b, const math::float3& c, const math::Oray& ray) {
	math::float3 e1 = b - a;
	math::float3 e2 = c - a;

	math::float3 pvec = math::cross(ray.direction, e2);

	float det = math::dot(e1, pvec);
	float inv_det = 1.f / det;

	math::float3 tvec = ray.origin - a;
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

inline void interpolate(const Vertex& a, const Vertex& b, const Vertex& c, math::float2 uv, math::float3& p, math::float3& n, math::float2& tc) {
	float w = 1.f - uv.x - uv.y;

	p  = w * a.p + uv.x * b.p + uv.y * c.p;
	n  = math::normalized(w * a.n + uv.x * b.n + uv.y * c.n);
	tc = w * a.uv + uv.x * b.uv + uv.y * c.uv;
}

inline void interpolate_data(const Vertex& a, const Vertex& b, const Vertex& c, math::float2 uv, math::float3& n, math::float3& t, math::float2& tc) {
	float w = 1.f - uv.x - uv.y;

	n  = math::normalized(w * a.n + uv.x * b.n + uv.y * c.n);
	t  = math::normalized(w * a.t + uv.x * b.t + uv.y * c.t);
	tc = w * a.uv + uv.x * b.uv + uv.y * c.uv;
}

inline math::float2 interpolate_uv(math::float2 a, math::float2 b, math::float2 c, math::float2 uv) {
	float w = 1.f - uv.x - uv.y;

	return w * a + uv.x * b + uv.y * c;
}

inline math::float3 normal(const math::float3& a, const math::float3& b, const math::float3& c) {
	math::float3 e1 = b - a;
	math::float3 e2 = c - a;

	return math::normalized(math::cross(e1, e2));
}

inline float area(const math::float3& a, const math::float3& b, const math::float3& c, const math::float3& scale) {
	math::float3 sa = scale * a;
	math::float3 sb = scale * b;
	math::float3 sc = scale * c;
	return 0.5f * math::length(math::cross(sb - sa, sc - sa));
}

}}}

