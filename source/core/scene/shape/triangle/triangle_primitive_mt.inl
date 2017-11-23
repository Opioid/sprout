#ifndef SU_CORE_SCENE_SHAPE_TRIANGLE_PRIMITIVE_MT_INL
#define SU_CORE_SCENE_SHAPE_TRIANGLE_PRIMITIVE_MT_INL

#include "triangle_primitive_mt.hpp"
#include "base/encoding/encoding.inl"
#include "base/math/ray.hpp"
#include "base/math/vector.inl"

namespace scene::shape::triangle {

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
		uv[0] = u;
		uv[1] = v;
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
	float w = 1.f - uv[0] - uv[1];

	p  = w * a.p + uv[0] * b.p + uv[1] * c.p;
	n  = math::normalize(w * a.n + uv[0] * b.n + uv[1] * c.n);
	tc = w * a.uv + uv[0] * b.uv + uv[1] * c.uv;
}

inline void Triangle_MT::interpolate(float2 uv, float3& p, float2& tc) const {
	float w = 1.f - uv[0] - uv[1];

	p  = w * a.p + uv[0] * b.p + uv[1] * c.p;
	tc = w * a.uv + uv[0] * b.uv + uv[1] * c.uv;
}

inline void Triangle_MT::interpolate(float2 uv, float3& p) const {
	float w = 1.f - uv[0] - uv[1];

	p = w * a.p + uv[0] * b.p + uv[1] * c.p;
}

inline void Triangle_MT::interpolate_data(float2 uv, float3& n, float3& t, float2& tc) const {
	float w = 1.f - uv[0] - uv[1];

	n  = math::normalize(w * a.n + uv[0] * b.n + uv[1] * c.n);
	t  = math::normalize(w * a.t + uv[0] * b.t + uv[1] * c.t);
	tc = w * a.uv + uv[0] * b.uv + uv[1] * c.uv;
}

inline float2 Triangle_MT::interpolate_uv(float2 uv) const {
	float w = 1.f - uv[0] - uv[1];

	return w * a.uv + uv[0] * b.uv + uv[1] * c.uv;
}

inline float3 Triangle_MT::normal() const {
	float3 e1 = b.p - a.p;
	float3 e2 = c.p - a.p;

	return math::normalize(math::cross(e1, e2));
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

static inline bool intersect(const Intersection_vertex_MT& a,
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

//	if (u < 0.f || u > 1.f) {
//		return false;
//	}

	float3 qvec = math::cross(tvec, e1);
	float v = math::dot(ray.direction, qvec) * inv_det;

//	if (v < 0.f || u + v > 1.f) {
//		return false;
//	}

	float hit_t = math::dot(e2, qvec) * inv_det;

//	if (hit_t > ray.min_t && hit_t < ray.max_t) {
//		ray.max_t = hit_t;
//		uv[0] = u;
//		uv[1] = v;
//		return true;
//	}

//	return false;

	uint8_t ca = static_cast<uint8_t>(u > 0.f);
	uint8_t cb = static_cast<uint8_t>(u < 1.f);
	uint8_t cc = static_cast<uint8_t>(v > 0.f);
	uint8_t cd = static_cast<uint8_t>(u + v < 1.f);
	uint8_t ce = static_cast<uint8_t>(hit_t > ray.min_t);
	uint8_t cf = static_cast<uint8_t>(hit_t < ray.max_t);

	if (0 != (ca & cb & cc & cd & ce & cf)) {
		ray.max_t = hit_t;
		uv[0] = u;
		uv[1] = v;
		return true;
	}

	return false;
}

static inline bool intersect(VVector origin, VVector direction, VVector min_t, Vector& max_t,
							 const Intersection_vertex_MT& a,
							 const Intersection_vertex_MT& b,
							 const Intersection_vertex_MT& c,
							 Vector& u_out, Vector& v_out) {
	using namespace math;

	Vector ap = simd::load_float4(a.p.v);
	Vector bp = simd::load_float4(b.p.v);
	Vector cp = simd::load_float4(c.p.v);

	Vector e1 = math::sub(bp, ap);
	Vector e2 = math::sub(cp, ap);

	Vector pvec = cross3(direction, e2);

	Vector inv_det = rcp1(dot3_1(e1, pvec));

	Vector tvec = math::sub(origin, ap);
	Vector u = math::mul1(dot3_1(tvec, pvec), inv_det);

	Vector qvec = cross3(tvec, e1);
	Vector v = math::mul1(dot3_1(direction, qvec), inv_det);

	Vector hit_t = math::mul1(dot3_1(e2, qvec), inv_det);

	Vector uv = math::add1(u, v);

	if (0 != (_mm_comige_ss(u, simd::Zero) &
			  _mm_comige_ss(simd::One, u) &
			  _mm_comige_ss(v, simd::Zero) &
			  _mm_comige_ss(simd::One, uv) &
			  _mm_comige_ss(hit_t, min_t) &
			  _mm_comige_ss(max_t, hit_t))) {
		max_t = hit_t;
		u_out = u;
		v_out = v;
		return true;
	}

	return false;
}

static inline bool intersect_p(const Intersection_vertex_MT& a,
							   const Intersection_vertex_MT& b,
							   const Intersection_vertex_MT& c,
							   const math::Ray& ray) {
	// Implementation A
/*	float3 e1 = b.p - a.p;
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
	// Implementation B

	float3 e1 = b.p - a.p;
	float3 e2 = c.p - a.p;

	float3 pvec = math::cross(ray.direction, e2);

	float det = math::dot(e1, pvec);
	float inv_det = 1.f / det;

	float3 tvec = ray.origin - a.p;
	float u = math::dot(tvec, pvec) * inv_det;

	float3 qvec = math::cross(tvec, e1);
	float v = math::dot(ray.direction, qvec) * inv_det;

	float hit_t = math::dot(e2, qvec) * inv_det;

	uint8_t ca = static_cast<uint8_t>(u > 0.f);
	uint8_t cb = static_cast<uint8_t>(u < 1.f);
	uint8_t cc = static_cast<uint8_t>(v > 0.f);
	uint8_t cd = static_cast<uint8_t>(u + v < 1.f);
	uint8_t ce = static_cast<uint8_t>(hit_t > ray.min_t);
	uint8_t cf = static_cast<uint8_t>(hit_t < ray.max_t);

	return 0 != (ca & cb & cc & cd & ce & cf);

}

static inline bool intersect_p(VVector origin, VVector direction,
							   VVector min_t, VVector max_t,
							   const Intersection_vertex_MT& a,
							   const Intersection_vertex_MT& b,
							   const Intersection_vertex_MT& c) {
	// Implementation C
	using namespace math;

	Vector ap = simd::load_float4(a.p.v);
	Vector bp = simd::load_float4(b.p.v);
	Vector cp = simd::load_float4(c.p.v);

	Vector e1 = math::sub(bp, ap);
	Vector e2 = math::sub(cp, ap);

	Vector pvec = cross3(direction, e2);

	Vector inv_det = rcp1(dot3_1(e1, pvec));

	Vector tvec = math::sub(origin, ap);
	Vector u = math::mul1(dot3_1(tvec, pvec), inv_det);

	Vector qvec = cross3(tvec, e1);
	Vector v = math::mul1(dot3_1(direction, qvec), inv_det);

	Vector hit_t = math::mul1(dot3_1(e2, qvec), inv_det);

	Vector uv = math::add1(u, v);

	return 0 != (_mm_comige_ss(u, simd::Zero) &
				 _mm_comige_ss(simd::One, u) &
				 _mm_comige_ss(v, simd::Zero) &
				 _mm_comige_ss(simd::One, uv) &
				 _mm_comige_ss(hit_t, min_t) &
				 _mm_comige_ss(max_t, hit_t));
}

static inline void interpolate_p(const Intersection_vertex_MT& a,
								 const Intersection_vertex_MT& b,
								 const Intersection_vertex_MT& c,
								 float2 uv, float3& p) {
	const float w = 1.f - uv[0] - uv[1];

	p = w * a.p + uv[0] * b.p + uv[1] * c.p;
}

static inline float area(const Intersection_vertex_MT& a,
						 const Intersection_vertex_MT& b,
						 const Intersection_vertex_MT& c) {
	return 0.5f * math::length(math::cross(b.p - a.p, c.p - a.p));
}

static inline float area(const Intersection_vertex_MT& a,
						 const Intersection_vertex_MT& b,
						 const Intersection_vertex_MT& c,
						 const float3& scale) {
	const float3 sa = scale * a.p;
	const float3 sb = scale * b.p;
	const float3 sc = scale * c.p;
	return 0.5f * math::length(math::cross(sb - sa, sc - sa));
}

static inline float2 interpolate_uv(const Shading_vertex_MT& a,
									const Shading_vertex_MT& b,
									const Shading_vertex_MT& c,
									float2 uv) {
	const float w = 1.f - uv[0] - uv[1];

	return w * a.uv + uv[0] * b.uv + uv[1] * c.uv;
}

static inline void interpolate_data(const Shading_vertex_MT& a,
									const Shading_vertex_MT& b,
									const Shading_vertex_MT& c,
									float2 uv,
									float3& n, float3& t, float2& tc) {
	const float w = 1.f - uv[0] - uv[1];

	n  = math::normalize(w * a.n + uv[0] * b.n + uv[1] * c.n);
	t  = math::normalize(w * a.t + uv[0] * b.t + uv[1] * c.t);
	tc = w * a.uv + uv[0] * b.uv + uv[1] * c.uv;
}

inline Shading_vertex_MTC::Shading_vertex_MTC(const packed_float3& n,
											  const packed_float3& t,
											  float2 uv) :
	n_u(n, uv[0]), t_v(t, uv[1]) {
	// Not too happy about handling degenerate tangents here (only one very special case even)
	if (0.f == t[0] && 0.f == t[1] && 0.f == t[2]) {
		t_v = float4(math::tangent(n_u.xyz()), uv[1]);
	}
}

static inline float2 interpolate_uv(const Shading_vertex_MTC& a,
									const Shading_vertex_MTC& b,
									const Shading_vertex_MTC& c,
									float2 uv) {
	const float w = 1.f - uv[0] - uv[1];

	return float2(w * a.n_u[3] + uv[0] * b.n_u[3] + uv[1] * c.n_u[3],
				  w * a.t_v[3] + uv[0] * b.t_v[3] + uv[1] * c.t_v[3]);
}

static inline float2 interpolate_uv(VVector u, VVector v,
									const Shading_vertex_MTC& a,
									const Shading_vertex_MTC& b,
									const Shading_vertex_MTC& c) {
	const Vector w = math::sub(math::sub(simd::One, u), v);

	const float3 auv(a.n_u[3], a.t_v[3], 0.f);
	Vector va = simd::load_float4(auv.v);

	const float3 buv(b.n_u[3], b.t_v[3], 0.f);
	Vector vb = simd::load_float4(buv.v);

	va = math::mul(w, va);
	vb = math::mul(u, vb);
	va = math::add(va, vb);

	const float3 cuv(c.n_u[3], c.t_v[3], 0.f);
	Vector vc = simd::load_float4(cuv.v);

	vc = math::mul(v, vc);
	Vector uv = math::add(va, vc);

	float3 r;
	simd::store_float4(r.v, uv);
	return r.xy();
}

static inline void interpolate_data(const Shading_vertex_MTC& a,
									const Shading_vertex_MTC& b,
									const Shading_vertex_MTC& c,
									float2 uv,
									float3& n, float3& t, float2& tc) {
	const float w = 1.f - uv[0] - uv[1];

	const float4 n_u = w * a.n_u + uv[0] * b.n_u + uv[1] * c.n_u;
	const float4 t_v = w * a.t_v + uv[0] * b.t_v + uv[1] * c.t_v;

	n  = math::normalize(n_u.xyz());
	t  = math::normalize(t_v.xyz());

	tc = float2(n_u[3], t_v[3]);
}

static inline void interpolate_data(VVector u, VVector v,
									const Shading_vertex_MTC& a,
									const Shading_vertex_MTC& b,
									const Shading_vertex_MTC& c,
									float3& n, float3& t, float2& tc) {
	const Vector w = math::sub(math::sub(simd::One, u), v);

	Vector va = math::mul(w, simd::load_float4(a.n_u.v));
	Vector vb = math::mul(u, simd::load_float4(b.n_u.v));
	va = math::add(va, vb);
	Vector vc = math::mul(v, simd::load_float4(c.n_u.v));
	Vector v0 = math::add(va, vc);

	Vector vn = math::normalized3(v0);
	simd::store_float4(n.v, vn);

	va = math::mul(w, simd::load_float4(a.t_v.v));
	vb = math::mul(u, simd::load_float4(b.t_v.v));
	va = math::add(va, vb);
	vc = math::mul(v, simd::load_float4(c.t_v.v));
	Vector v1 = math::add(va, vc);

	Vector vt = math::normalized3(v1);
	simd::store_float4(t.v, vt);

	v0 = SU_MUX_HIGH(v0, v1);
	float4 r;
	simd::store_float4(r.v, v0);
	tc[0] = r[3];
	tc[1] = r[1];
}

static inline void interpolate_data(VVector u, VVector v,
									const Shading_vertex_MTC& a,
									const Shading_vertex_MTC& b,
									const Shading_vertex_MTC& c,
									Vector& n, Vector& t, float2& tc) {
	const Vector w = math::sub(math::sub(simd::One, u), v);

	Vector va = math::mul(w, simd::load_float4(a.n_u.v));
	Vector vb = math::mul(u, simd::load_float4(b.n_u.v));
	va = math::add(va, vb);
	Vector vc = math::mul(v, simd::load_float4(c.n_u.v));
	Vector v0 = math::add(va, vc);

	n = math::normalized3(v0);

	va = math::mul(w, simd::load_float4(a.t_v.v));
	vb = math::mul(u, simd::load_float4(b.t_v.v));
	va = math::add(va, vb);
	vc = math::mul(v, simd::load_float4(c.t_v.v));
	Vector v1 = math::add(va, vc);

	t = math::normalized3(v1);

	v0 = SU_MUX_HIGH(v0, v1);
	float4 r;
	simd::store_float4(r.v, v0);
	tc[0] = r[3];
	tc[1] = r[1];
}

inline float xnorm_to_float(int16_t xnorm) {
	return static_cast<float>(xnorm) / 511.f;
}

inline int16_t float_to_xnorm(float x) {
	return static_cast<int16_t>(x * 511.f);
}

inline float4 snorm16_to_float(short4 v) {
	return float4(encoding::snorm16_to_float(v[0]),
				  encoding::snorm16_to_float(v[1]),
				  encoding::snorm16_to_float(v[2]),
				  xnorm_to_float(v[3]));
}

inline short4 float_to_snorm16(const float3& v, float s) {
	return short4(encoding::float_to_snorm16(v[0]),
				  encoding::float_to_snorm16(v[1]),
				  encoding::float_to_snorm16(v[2]),
				  float_to_xnorm(s));
}

inline short4 float_to_snorm16(const packed_float3& v, float s) {
	return short4(encoding::float_to_snorm16(v[0]),
				  encoding::float_to_snorm16(v[1]),
				  encoding::float_to_snorm16(v[2]),
				  float_to_xnorm(s));
}

inline Shading_vertex_MTCC::Shading_vertex_MTCC(const packed_float3& n,
												const packed_float3& t,
												float2 uv) :
	n_u(float_to_snorm16(n, uv[0])),
	t_v(float_to_snorm16(t, uv[1])) {
	// Not too happy about handling degenerate tangents here (only one very special case even)
	if (0.f == t[0] && 0.f == t[1] && 0.f == t[2]) {
		t_v = float_to_snorm16(math::tangent(float3(n)), uv[1]);
	}
}

inline float2 interpolate_uv(const Shading_vertex_MTCC& a,
							 const Shading_vertex_MTCC& b,
							 const Shading_vertex_MTCC& c,
							 float2 uv) {
	float w = 1.f - uv[0] - uv[1];

	float au = xnorm_to_float(a.n_u[3]);
	float av = xnorm_to_float(a.t_v[3]);
	float bu = xnorm_to_float(b.n_u[3]);
	float bv = xnorm_to_float(b.t_v[3]);
	float cu = xnorm_to_float(c.n_u[3]);
	float cv = xnorm_to_float(c.t_v[3]);

	return float2(w * au + uv[0] * bu + uv[1] * cu,
				  w * av + uv[0] * bv + uv[1] * cv);
}

inline void interpolate_data(const Shading_vertex_MTCC& a,
							 const Shading_vertex_MTCC& b,
							 const Shading_vertex_MTCC& c,
							 float2 uv,
							 float3& n, float3& t, float2& tc) {
	float w = 1.f - uv[0] - uv[1];

	float4 an_u = snorm16_to_float(a.n_u);
	float4 at_v = snorm16_to_float(a.t_v);
	float4 bn_u = snorm16_to_float(b.n_u);
	float4 bt_v = snorm16_to_float(b.t_v);
	float4 cn_u = snorm16_to_float(c.n_u);
	float4 ct_v = snorm16_to_float(c.t_v);

	float4 n_u = w * an_u + uv[0] * bn_u + uv[1] * cn_u;
	float4 t_v = w * at_v + uv[0] * bt_v + uv[1] * ct_v;

	n  = math::normalize(n_u.xyz());
	t  = math::normalize(t_v.xyz());

	tc = float2(n_u[3], t_v[3]);
}

inline Vertex_MTC::Vertex_MTC(const packed_float3& p,
							  const packed_float3& n,
							  const packed_float3& t,
							  float2 uv) :
	p(p), n_u(n, uv[0]), t_v(t, uv[1]) {
	// Not too happy about handling degenerate tangents here (only one very special case even)
	if (0.f == t[0] && 0.f == t[1] && 0.f == t[2]) {
		t_v = float4(math::tangent(n_u.xyz()), uv[1]);
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
		uv[0] = u;
		uv[1] = v;
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
	float w = 1.f - uv[0] - uv[1];

	return float2(w * a.n_u[3] + uv[0] * b.n_u[3] + uv[1] * c.n_u[3],
				  w * a.t_v[3] + uv[0] * b.t_v[3] + uv[1] * c.t_v[3]);
}

inline void interpolate_p_uv(const Vertex_MTC& a,
							 const Vertex_MTC& b,
							 const Vertex_MTC& c,
							 float2 uv,
							 float3& p, float2& tc) {
	float w = 1.f - uv[0] - uv[1];

	p = w * a.p + uv[0] * b.p + uv[1] * c.p;

	tc[0] = w * a.n_u[3] + uv[0] * b.n_u[3] + uv[1] * c.n_u[3];
	tc[1] = w * a.t_v[3] + uv[0] * b.t_v[3] + uv[1] * c.t_v[3];
}

inline void interpolate_data(const Vertex_MTC& a,
							 const Vertex_MTC& b,
							 const Vertex_MTC& c,
							 float2 uv,
							 float3& n, float3& t, float2& tc) {
	float w = 1.f - uv[0] - uv[1];

	float4 n_u = w * a.n_u + uv[0] * b.n_u + uv[1] * c.n_u;
	float4 t_v = w * a.t_v + uv[0] * b.t_v + uv[1] * c.t_v;

	n  = math::normalize(n_u.xyz());
	t  = math::normalize(t_v.xyz());

	tc = float2(n_u[3], t_v[3]);
}

inline float area(const Vertex_MTC& a,
				  const Vertex_MTC& b,
				  const Vertex_MTC& c) {
	return 0.5f * math::length(math::cross(b.p - a.p, c.p - a.p));
}

inline float area(const Vertex_MTC& a,
				  const Vertex_MTC& b,
				  const Vertex_MTC& c,
				  const float3& scale) {
	float3 sa = scale * a.p;
	float3 sb = scale * b.p;
	float3 sc = scale * c.p;
	return 0.5f * math::length(math::cross(sb - sa, sc - sa));
}

}

#endif
