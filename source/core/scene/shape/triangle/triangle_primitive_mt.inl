#pragma once

#include "triangle_primitive_mt.hpp"
#include "base/encoding/encoding.inl"
#include "base/math/simd/simd_vector.inl"

#include <iostream>

namespace scene { namespace shape { namespace triangle {

inline Triangle_MT::Triangle_MT(const shape::Vertex& a,
								const shape::Vertex& b,
								const shape::Vertex& c,
								float bitangent_sign,
								uint32_t material_index) :
	a(a), b(b), c(c), bitangent_sign(bitangent_sign), material_index(material_index) {}

inline Triangle_MT::Vertex::Vertex(const shape::Vertex& v) : p(v.p), n(v.n), t(v.t), uv(v.uv) {}

inline bool Triangle_MT::intersect(math::Ray& ray, float2& uv) const {
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

inline bool Triangle_MT::intersect_p(const math::Ray& ray) const {
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

inline void Triangle_MT::interpolate(float2 uv, float3& p, float3& n, float2& tc) const {
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

inline void Triangle_MT::interpolate_data(float2 uv, float3& n, float3& t, float2& tc) const {
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

inline float Triangle_MT::area(float3_p scale) const {
	float3 sa = scale * a.p;
	float3 sb = scale * b.p;
	float3 sc = scale * c.p;
	return 0.5f * math::length(math::cross(sb - sa, sc - sa));
}

inline bool intersect(const Intersection_vertex_MT& a,
					  const Intersection_vertex_MT& b,
					  const Intersection_vertex_MT& c,
					  math::Ray& ray, float2& uv) {
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
						const math::Ray& ray) {
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

inline bool intersect_p(math::simd::FVector origin,
						math::simd::FVector direction,
						math::simd::FVector min_t,
						math::simd::FVector max_t,
						const Intersection_vertex_MT& a,
						const Intersection_vertex_MT& b,
						const Intersection_vertex_MT& c) {
	/*
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
	*/

	using namespace math;

	simd::Vector ap = simd::load_float3(a.p);
	simd::Vector bp = simd::load_float3(b.p);
	simd::Vector cp = simd::load_float3(c.p);

	simd::Vector e1 = simd::sub3(bp, ap);
	simd::Vector e2 = simd::sub3(cp, ap);

	simd::Vector pvec = simd::cross3(direction, e2);

	simd::Vector inv_det = simd::rcp1(simd::dot3(e1, pvec));

	simd::Vector tvec = simd::sub3(origin, ap);
	simd::Vector u = simd::mul1(simd::dot3(tvec, pvec), inv_det);

	bool ur = 0 != (_mm_comige_ss(u, simd::Zero) &
					_mm_comige_ss(simd::One, u));

	if (!ur) {
		return false;
	}

	simd::Vector qvec = simd::cross3(tvec, e1);
	simd::Vector v = simd::mul1(simd::dot3(direction, qvec), inv_det);

	simd::Vector hit_t = simd::mul1(simd::dot3(e2, qvec), inv_det);

	simd::Vector uv = simd::add1(u, v);

	return 0 != (/*_mm_comige_ss(u, simd::Zero) &*/
				 _mm_comige_ss(v, simd::Zero) &
				 _mm_comige_ss(simd::One, uv) &
				 _mm_comige_ss(hit_t, min_t) &
				 _mm_comige_ss(max_t, hit_t));
}

inline void interpolate_p(const Intersection_vertex_MT& a,
						  const Intersection_vertex_MT& b,
						  const Intersection_vertex_MT& c,
						  float2 uv, float3& p) {
	float w = 1.f - uv.x - uv.y;

	p = w * a.p + uv.x * b.p + uv.y * c.p;
}

inline float area(const Intersection_vertex_MT& a,
				  const Intersection_vertex_MT& b,
				  const Intersection_vertex_MT& c) {
	return 0.5f * math::length(math::cross(b.p - a.p, c.p - a.p));
}

inline float area(const Intersection_vertex_MT& a,
				  const Intersection_vertex_MT& b,
				  const Intersection_vertex_MT& c,
				  float3_p scale) {
	float3 sa = scale * a.p;
	float3 sb = scale * b.p;
	float3 sc = scale * c.p;
	return 0.5f * math::length(math::cross(sb - sa, sc - sa));
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

inline Shading_vertex_MTC::Shading_vertex_MTC(const math::packed_float3& n,
											  const math::packed_float3& t,
											  float2 uv) :
	n_u(n, uv.x), t_v(t, uv.y) {
	// Not too happy about handling degenerate tangents here (only one very special case even)
	if (0.f == t.x &&  0.f == t.y &&  0.f == t.z) {
		t_v = float4(math::tangent(n_u.xyz), uv.y);
	}
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

	float4 n_u = w * a.n_u + uv.x * b.n_u + uv.y * c.n_u;
	float4 t_v = w * a.t_v + uv.x * b.t_v + uv.y * c.t_v;

	n  = math::normalized(n_u.xyz);
	t  = math::normalized(t_v.xyz);

	tc = float2(n_u.w, t_v.w);
}

inline float xnorm_to_float(int16_t xnorm) {
	return static_cast<float>(xnorm) / 511.f;
}

inline int16_t float_to_xnorm(float x) {
	return static_cast<int16_t>(x * 511.f);
}

inline float4 snorm16_to_float(short4 v) {
	return float4(encoding::snorm16_to_float(v.x),
				  encoding::snorm16_to_float(v.y),
				  encoding::snorm16_to_float(v.z),
				  xnorm_to_float(v.w));
}

inline short4 float_to_snorm16(float3_p v, float s) {
	return short4(encoding::float_to_snorm16(v.x),
				  encoding::float_to_snorm16(v.y),
				  encoding::float_to_snorm16(v.z),
				  float_to_xnorm(s));
}

inline short4 float_to_snorm16(const math::packed_float3& v, float s) {
	return short4(encoding::float_to_snorm16(v.x),
				  encoding::float_to_snorm16(v.y),
				  encoding::float_to_snorm16(v.z),
				  float_to_xnorm(s));
}

inline Shading_vertex_MTCC::Shading_vertex_MTCC(const math::packed_float3& n,
												const math::packed_float3& t,
												float2 uv) :
	n_u(float_to_snorm16(n, uv.x)),
	t_v(float_to_snorm16(t, uv.y)) {


	float u = xnorm_to_float(n_u.w);
	float du = std::abs(uv.x - u);

	if (du > 0.1f) {
		std::cout << uv.x << " vs " << u << std::endl;
	}

	// Not too happy about handling degenerate tangents here (only one very special case even)
	if (0.f == t.x &&  0.f == t.y &&  0.f == t.z) {
		t_v = float_to_snorm16(math::tangent(float3(n)), uv.y);
	}
}

inline float2 interpolate_uv(const Shading_vertex_MTCC& a,
							 const Shading_vertex_MTCC& b,
							 const Shading_vertex_MTCC& c,
							 float2 uv) {
	float w = 1.f - uv.x - uv.y;

	float au = xnorm_to_float(a.n_u.w);
	float av = xnorm_to_float(a.t_v.w);
	float bu = xnorm_to_float(b.n_u.w);
	float bv = xnorm_to_float(b.t_v.w);
	float cu = xnorm_to_float(c.n_u.w);
	float cv = xnorm_to_float(c.t_v.w);

	return float2(w * au + uv.x * bu + uv.y * cu,
				  w * av + uv.x * bv + uv.y * cv);
}

inline void interpolate_data(const Shading_vertex_MTCC& a,
							 const Shading_vertex_MTCC& b,
							 const Shading_vertex_MTCC& c,
							 float2 uv,
							 float3& n, float3& t, float2& tc) {
	float w = 1.f - uv.x - uv.y;

	float4 an_u = snorm16_to_float(a.n_u);
	float4 at_v = snorm16_to_float(a.t_v);
	float4 bn_u = snorm16_to_float(b.n_u);
	float4 bt_v = snorm16_to_float(b.t_v);
	float4 cn_u = snorm16_to_float(c.n_u);
	float4 ct_v = snorm16_to_float(c.t_v);

	float4 n_u = w * an_u + uv.x * bn_u + uv.y * cn_u;
	float4 t_v = w * at_v + uv.x * bt_v + uv.y * ct_v;

	n  = math::normalized(n_u.xyz);
	t  = math::normalized(t_v.xyz);

	tc = float2(n_u.w, t_v.w);
}

inline Vertex_MTC::Vertex_MTC(const math::packed_float3& p,
							  const math::packed_float3& n,
							  const math::packed_float3& t,
							  float2 uv) :
	p(p), n_u(n, uv.x), t_v(t, uv.y) {
	// Not too happy about handling degenerate tangents here (only one very special case even)
	if (0.f == t.x &&  0.f == t.y &&  0.f == t.z) {
		t_v = float4(math::tangent(n_u.xyz), uv.y);
	}
}

inline bool intersect(const Vertex_MTC& a,
					  const Vertex_MTC& b,
					  const Vertex_MTC& c,
					  math::Ray& ray, float2& uv) {
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

inline bool intersect_p(const Vertex_MTC& a,
						const Vertex_MTC& b,
						const Vertex_MTC& c,
						const math::Ray& ray) {
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

inline float2 interpolate_uv(const Vertex_MTC& a,
							 const Vertex_MTC& b,
							 const Vertex_MTC& c,
							 float2 uv) {
	float w = 1.f - uv.x - uv.y;

	return float2(w * a.n_u.w + uv.x * b.n_u.w + uv.y * c.n_u.w,
				  w * a.t_v.w + uv.x * b.t_v.w + uv.y * c.t_v.w);
}

inline void interpolate_p_uv(const Vertex_MTC& a,
							 const Vertex_MTC& b,
							 const Vertex_MTC& c,
							 float2 uv,
							 float3& p, float2& tc) {
	float w = 1.f - uv.x - uv.y;

	p = w * a.p + uv.x * b.p + uv.y * c.p;

	tc.x = w * a.n_u.w + uv.x * b.n_u.w + uv.y * c.n_u.w;
	tc.y = w * a.t_v.w + uv.x * b.t_v.w + uv.y * c.t_v.w;
}

inline void interpolate_data(const Vertex_MTC& a,
							 const Vertex_MTC& b,
							 const Vertex_MTC& c,
							 float2 uv,
							 float3& n, float3& t, float2& tc) {
	float w = 1.f - uv.x - uv.y;

	float4 n_u = w * a.n_u + uv.x * b.n_u + uv.y * c.n_u;
	float4 t_v = w * a.t_v + uv.x * b.t_v + uv.y * c.t_v;

	n  = math::normalized(n_u.xyz);
	t  = math::normalized(t_v.xyz);

	tc = float2(n_u.w, t_v.w);
}

inline float area(const Vertex_MTC& a,
				  const Vertex_MTC& b,
				  const Vertex_MTC& c) {
	return 0.5f * math::length(math::cross(b.p - a.p, c.p - a.p));
}

inline float area(const Vertex_MTC& a,
				  const Vertex_MTC& b,
				  const Vertex_MTC& c,
				  float3_p scale) {
	float3 sa = scale * a.p;
	float3 sb = scale * b.p;
	float3 sc = scale * c.p;
	return 0.5f * math::length(math::cross(sb - sa, sc - sa));
}

}}}
