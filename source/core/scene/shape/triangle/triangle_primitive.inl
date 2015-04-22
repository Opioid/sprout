#include "triangle_primitive.hpp"
#include "triangle_intersection.hpp"

namespace scene { namespace shape { namespace triangle {

inline bool Triangle::intersect(const math::Oray& ray, Coordinates& coordinates) const {
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
		coordinates.t = hit_t;
		coordinates.u = u;
		coordinates.v = v;
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

inline void Triangle::interpolate(float u, float v, math::float3& n, math::float3& t, math::float2& uv) const {
	float w = 1.f - u - v;

	n  = math::normalized(w * a.n + u * b.n + v * c.n);
	t  = math::normalized(w * a.t + u * b.t + v * c.t);
	uv = w * a.uv + u * b.uv + v * c.uv;
}

}}}

