#include "triangle_mesh.hpp"
#include "base/math/aabb.inl"
#include "base/math/distribution/distribution_1d.inl"
#include "base/math/matrix3x3.inl"
#include "base/math/matrix4x4.inl"
#include "base/math/sampling.inl"
#include "base/math/vector3.inl"
#include "base/memory/buffer.hpp"
#include "bvh/triangle_bvh_tree.inl"
#include "sampler/sampler.hpp"
#include "scene/entity/composed_transformation.inl"
#include "scene/light/light.hpp"
#include "scene/light/light_tree_builder.hpp"
#include "scene/ray_offset.inl"
#include "scene/scene_ray.inl"
#include "scene/shape/shape_intersection.hpp"
#include "scene/shape/shape_sample.hpp"
#include "triangle_intersection.hpp"
#ifdef SU_DEBUG
#include "scene/shape/shape_test.hpp"
#endif
#include "base/debug/assert.hpp"

namespace scene::shape::triangle {

Part::~Part() {
    delete[] cones;
    delete[] aabbs;
    delete[] triangle_mapping;
}

void Part::init(uint32_t part, bool two_sided, bvh::Tree const& tree, light::Tree_builder& builder,
                Threads& threads) {
    if (nullptr != triangle_mapping) {
        return;
    }

    uint32_t const num = num_triangles;

    memory::Buffer<float> areas(num);

    triangle_mapping = new uint32_t[num];

    for (uint32_t t = 0, mt = 0, len = tree.num_triangles(); t < len; ++t) {
        if (tree.triangle_part(t) == part) {
            areas[mt] = tree.triangle_area(t);

            triangle_mapping[mt] = t;

            ++mt;
        }
    }

    distribution.init(areas, num);

    aabbs = new AABB[num];

    cones = new float4[num];

    AABB bb(Empty_AABB);

    float3 dominant_axis(0.f);

    float const a = 1.f / distribution.integral();

    for (uint32_t i = 0; i < num; ++i) {
        uint32_t const t = triangle_mapping[i];

        float3 va;
        float3 vb;
        float3 vc;
        tree.triangle(t, va, vb, vc);

        AABB box(Empty_AABB);
        box.insert(va);
        box.insert(vb);
        box.insert(vc);

        bb.merge_assign(box);

        box.cache_radius();

        float const area = tree.triangle_area(t);

        box.bounds[1][3] = area;

        aabbs[i] = box;

        float3 const n = tree.triangle_normal(t);

        cones[i] = float4(n, 1.f);

        dominant_axis += a * area * n;
    }

    dominant_axis = normalize(dominant_axis);

    float angle = 0.f;

    for (uint32_t i = 0; i < num; ++i) {
        uint32_t const t = triangle_mapping[i];

        float3 const n = tree.triangle_normal(t);
        float const  c = dot(dominant_axis, n);

        SOFT_ASSERT(std::isfinite(c));

        angle = std::max(angle, std::acos(c));
    }

    aabb = bb;
    cone = float4(dominant_axis, std::cos(angle));

    two_sided_ = two_sided;

    builder.build(light_tree, *this, threads);
}

light::Pick Part::sample(float3_p p, float3_p n, bool total_sphere, float r) const {
    auto const pick = light_tree.random_light(p, n, total_sphere, r, *this);

    float const relative_primitive_area = distribution.pdf(pick.offset);

    return {triangle_mapping[pick.offset], pick.pdf / relative_primitive_area};
}

float Part::pdf(float3_p p, float3_p n, bool total_sphere, uint32_t id) const {
    float const pdf = light_tree.pdf(p, n, total_sphere, id, *this);

    float const relative_primitive_area = distribution.pdf(id);

    return pdf / relative_primitive_area;
}

math::Distribution_1D::Discrete Part::sample(float r) const {
    auto const result = distribution.sample_discrete(r);
    return {triangle_mapping[result.offset], result.pdf};
}

AABB const& Part::light_aabb(uint32_t light) const {
    SOFT_ASSERT(light < num_triangles);

    return aabbs[light];
}

float4_p Part::light_cone(uint32_t light) const {
    SOFT_ASSERT(light < num_triangles);

    return cones[light];
}

bool Part::light_two_sided(uint32_t /*light*/) const {
    return two_sided_;
}

float Part::light_power(uint32_t light) const {
    SOFT_ASSERT(light < num_triangles);

    return aabbs[light].bounds[1][3];
}

Mesh::Mesh()
    : Shape(Properties(Property::Complex, Property::Finite)),
      parts_(nullptr),
      primitive_mapping_(nullptr) {}

Mesh::~Mesh() {
    delete[] primitive_mapping_;
    delete[] parts_;
}

bvh::Tree& Mesh::tree() {
    return tree_;
}

void Mesh::allocate_parts(uint32_t num_parts) {
    tree_.allocate_parts(num_parts);

    parts_ = new Part[num_parts];
}

void Mesh::set_material_for_part(uint32_t part, uint32_t material) {
    parts_[part].material = material;
}

float3 Mesh::object_to_texture_point(float3_p p) const {
    AABB const aabb = tree_.aabb();
    return (p - aabb.bounds[0]) / aabb.extent();
}

float3 Mesh::object_to_texture_vector(float3_p v) const {
    return v / tree_.aabb().extent();
}

AABB Mesh::transformed_aabb(float4x4 const& m) const {
    return tree_.aabb().transform(m);
}

AABB Mesh::transformed_part_aabb(uint32_t part, float4x4 const& m) const {
    return parts_[part].aabb.transform(m);
}

uint32_t Mesh::num_parts() const {
    return tree_.num_parts();
}

uint32_t Mesh::num_materials() const {
    uint32_t id = 0;

    for (uint32_t i = 0, len = num_parts(); i < len; ++i) {
        id = std::max(id, parts_[i].material);
    }

    return id + 1;
}

uint32_t Mesh::part_id_to_material_id(uint32_t part) const {
    return parts_[part].material;
}

bool Mesh::intersect(Ray& ray, Transformation const& trafo, Node_stack& nodes,
                     shape::Intersection& isec) const {
    Simd4x4f const world_to_object(trafo.world_to_object);

    Simd3f const ray_origin    = transform_point(world_to_object, Simd3f(ray.origin));
    Simd3f const ray_direction = transform_vector(world_to_object, Simd3f(ray.direction));

    scalar const ray_min_t(ray.min_t());
    scalar       ray_max_t(ray.max_t());

    if (Intersection pi;
        tree_.intersect(ray_origin, ray_direction, ray_min_t, ray_max_t, nodes, pi)) {
        ray.max_t() = ray_max_t.x();

        Simd3f p = tree_.interpolate_p(pi.u, pi.v, pi.index);

        Simd4x4f const object_to_world(trafo.object_to_world());

        Simd3f p_w = transform_point(object_to_world, p);

        Simd3f n;
        Simd3f t;
        float2 uv;
        tree_.interpolate_triangle_data(pi.u, pi.v, pi.index, n, t, uv);

        Simd3f geo_n = tree_.triangle_normal_v(pi.index);

        Simd3f bitangent_sign(tree_.triangle_bitangent_sign(pi.index));

        uint32_t part = tree_.triangle_part(pi.index);

        Simd3x3f rotation(trafo.rotation);

        Simd3f geo_n_w = transform_vector(rotation, geo_n);
        Simd3f n_w     = transform_vector(rotation, n);
        Simd3f t_w     = transform_vector(rotation, t);
        Simd3f b_w     = bitangent_sign * cross(n_w, t_w);

        isec.p         = float3(p_w);
        isec.t         = float3(t_w);
        isec.b         = float3(b_w);
        isec.n         = float3(n_w);
        isec.geo_n     = float3(geo_n_w);
        isec.uv        = uv;
        isec.part      = part;
        isec.primitive = pi.index;

        SOFT_ASSERT(testing::check(isec, trafo, ray));

        return true;
    }

    return false;
}

bool Mesh::intersect_nsf(Ray& ray, Transformation const& trafo, Node_stack& nodes,
                         shape::Intersection& isec) const {
    Simd4x4f const world_to_object(trafo.world_to_object);

    Simd3f const ray_origin    = transform_point(world_to_object, Simd3f(ray.origin));
    Simd3f const ray_direction = transform_vector(world_to_object, Simd3f(ray.direction));

    scalar const ray_min_t(ray.min_t());
    scalar       ray_max_t(ray.max_t());

    if (Intersection pi;
        tree_.intersect(ray_origin, ray_direction, ray_min_t, ray_max_t, nodes, pi)) {
        ray.max_t() = ray_max_t.x();

        Simd3f p = tree_.interpolate_p(pi.u, pi.v, pi.index);

        Simd4x4f const object_to_world(trafo.object_to_world());

        Simd3f p_w = transform_point(object_to_world, p);

        float2 const uv = tree_.interpolate_triangle_uv(pi.u, pi.v, pi.index);

        Simd3f geo_n = tree_.triangle_normal_v(pi.index);

        uint32_t part = tree_.triangle_part(pi.index);

        Simd3x3f rotation(trafo.rotation);

        Simd3f geo_n_w = transform_vector(rotation, geo_n);

        isec.p         = float3(p_w);
        isec.geo_n     = float3(geo_n_w);
        isec.uv        = uv;
        isec.part      = part;
        isec.primitive = pi.index;

        return true;
    }

    return false;
}

bool Mesh::intersect(Ray& ray, Transformation const& trafo, Node_stack& nodes,
                     Normals& normals) const {
    Simd4x4f const world_to_object(trafo.world_to_object);

    Simd3f const ray_origin    = transform_point(world_to_object, Simd3f(ray.origin));
    Simd3f const ray_direction = transform_vector(world_to_object, Simd3f(ray.direction));

    scalar const ray_min_t(ray.min_t());
    scalar       ray_max_t(ray.max_t());

    if (Intersection pi;
        tree_.intersect(ray_origin, ray_direction, ray_min_t, ray_max_t, nodes, pi)) {
        ray.max_t() = ray_max_t.x();

        Simd3f n = tree_.interpolate_shading_normal(pi.u, pi.v, pi.index);

        Simd3f geo_n = tree_.triangle_normal_v(pi.index);

        Simd3x3f rotation(trafo.rotation);

        Simd3f geo_n_w = transform_vector(rotation, geo_n);
        Simd3f n_w     = transform_vector(rotation, n);

        normals.geo_n = float3(geo_n_w);
        normals.n     = float3(n_w);

        return true;
    }

    return false;
}

bool Mesh::intersect_p(Ray const& ray, Transformation const& trafo, Node_stack& nodes) const {
    //	ray tray;
    //	tray.origin = transform_point(ray.origin, trafo.world_to_object);
    //	tray.set_direction(transform_vector(ray.direction, trafo.world_to_object));
    //	tray.min_t = ray.min_t;
    //	tray.max_t = ray.max_t;

    //	return tree_.intersect_p(tray, nodes);

    Simd4x4f const world_to_object(trafo.world_to_object);

    Simd3f const ray_origin    = transform_point(world_to_object, Simd3f(ray.origin));
    Simd3f const ray_direction = transform_vector(world_to_object, Simd3f(ray.direction));

    scalar const ray_min_t(ray.min_t());
    scalar const ray_max_t(ray.max_t());

    return tree_.intersect_p(ray_origin, ray_direction, ray_min_t, ray_max_t, nodes);
}

float Mesh::visibility(Ray const& ray, Transformation const& trafo, uint32_t entity, Filter filter,
                       Worker& worker) const {
    math::ray tray(trafo.world_to_object_point(ray.origin),
                   trafo.world_to_object_vector(ray.direction), ray.min_t(), ray.max_t());

    return tree_.visibility(tray, entity, filter, worker);
}

bool Mesh::thin_absorption(Ray const& ray, Transformation const& trafo, uint32_t entity,
                           Filter filter, Worker& worker, float3& ta) const {
    math::ray tray(trafo.world_to_object_point(ray.origin),
                   trafo.world_to_object_vector(ray.direction), ray.min_t(), ray.max_t());

    return tree_.absorption(tray, entity, filter, worker, ta);
}

bool Mesh::sample(uint32_t part, float3_p p, float3_p n, Transformation const& trafo, float area,
                  bool two_sided, bool total_sphere, Sampler& sampler, RNG& rng, uint32_t sampler_d,
                  Sample_to& sample) const {
    float const  r  = sampler.sample_1D(rng, sampler_d);
    float2 const r2 = sampler.sample_2D(rng, sampler_d);

    float3 const op = trafo.world_to_object_point(p);
    float3 const on = trafo.world_to_object_normal(n);
    auto const   s  = parts_[part].sample(op, on, total_sphere, r);

    float3 sv;
    float2 tc;
    tree_.sample(s.offset, r2, sv, tc);
    float3 const v  = trafo.object_to_world_point(sv);
    float3 const sn = tree_.triangle_normal(s.offset);
    float3       wn = transform_vector(trafo.rotation, sn);

    if (two_sided && dot(wn, v - p) > 0.f) {
        wn *= -1.f;
    }

    float3 const axis = offset_ray(v, wn) - p;
    float const  sl   = squared_length(axis);
    float const  d    = std::sqrt(sl);
    float3 const dir  = axis / d;
    float const  c    = -dot(wn, dir);

    if (c < Dot_min) {
        return false;
    }

    float const pdf = sl / (c * area);

    sample = Sample_to(dir, wn, float3(tc), pdf * s.pdf, d);

#ifdef SU_DEBUG
    uint32_t const pm = primitive_mapping_[s.offset];

    float const guessed_pdf = parts_[part].pdf(op, on, total_sphere, pm);

    float const diff = std::abs(guessed_pdf - s.pdf);

    SOFT_ASSERT(diff < 1e-8f);

    if (diff >= 1e-8f) {
        std::cout << "problem " << s.pdf << "  " << guessed_pdf << std::endl;
    }
#endif

    return true;
}

bool Mesh::sample(uint32_t part, Transformation const& trafo, float area, bool /*two_sided*/,
                  Sampler& sampler, RNG& rng, uint32_t sampler_d, float2 importance_uv,
                  AABB const& /*bounds*/, Sample_from& sample) const {
    float const r = sampler.sample_1D(rng, sampler_d);
    auto const  s = parts_[part].sample(r);

    float2 const r0 = sampler.sample_2D(rng, sampler_d);

    float3 sv;
    float2 tc;
    tree_.sample(s.offset, r0, sv, tc);
    float3 const ws = trafo.object_to_world_point(sv);

    float3 const sn = tree_.triangle_normal(s.offset);
    float3 const wn = transform_vector(trafo.rotation, sn);

    auto const [x, y] = orthonormal_basis(wn);

    float3 const dir = sample_oriented_hemisphere_cosine(importance_uv, x, y, wn);

    sample = Sample_from(offset_ray(ws, wn), wn, dir, tc, importance_uv, 1.f / (Pi * area));

    return true;
}

float Mesh::pdf(Ray const& ray, float3_p n, shape::Intersection const& isec,
                Transformation const& trafo, float area, bool two_sided, bool total_sphere) const {
    float c = -dot(isec.geo_n, ray.direction);

    if (two_sided) {
        c = std::abs(c);
    }

    float const sl  = ray.max_t() * ray.max_t();
    float const pdf = sl / (c * area);

    float3 const op = trafo.world_to_object_point(ray.origin);
    float3 const on = trafo.world_to_object_normal(n);

    uint32_t const pm = primitive_mapping_[isec.primitive];

    float const tri_pdf = parts_[isec.part].pdf(op, on, total_sphere, pm);

    return pdf * tri_pdf;
}

float Mesh::pdf_volume(Ray const& /*ray*/, shape::Intersection const& /*isec*/,
                       Transformation const& /*trafo*/, float /*area*/) const {
    return 0.f;
}

bool Mesh::sample(uint32_t /*part*/, float3_p /*p*/, float2 /*uv*/, Transformation const& /*trafo*/,
                  float /*area*/, bool /*two_sided*/, Sample_to& /*sample*/) const {
    return false;
}

bool Mesh::sample(uint32_t /*part*/, float3_p /*p*/, float3_p /*uvw*/,
                  Transformation const& /*trafo*/, float /*volume*/, Sample_to& /*sample*/) const {
    return false;
}

bool Mesh::sample(uint32_t /*part*/, float2 /*uv*/, Transformation const& /*trafo*/, float /*area*/,
                  bool /*two_sided*/, float2 /*importance_uv*/, AABB const& /*bounds*/,
                  Sample_from& /*sample*/) const {
    return false;
}

float Mesh::pdf_uv(Ray const& /*ray*/, shape::Intersection const& /*isec*/,
                   Transformation const& /*trafo*/, float /*area*/, bool /*two_sided*/) const {
    return 0.f;
}

float Mesh::uv_weight(float2 /*uv*/) const {
    return 1.f;
}

float Mesh::area(uint32_t part, float3_p scale) const {
    // HACK: This only really works for uniform scales!
    return parts_[part].distribution.integral() * (scale[0] * scale[1]);
}

float Mesh::volume(uint32_t /*part*/, float3_p /*scale*/) const {
    // HACK: This only really works for uniform scales!
    return 1.f;
}

Shape::Differential_surface Mesh::differential_surface(uint32_t primitive) const {
    float3 pa;
    float3 pb;
    float3 pc;

    float2 uva;
    float2 uvb;
    float2 uvc;

    tree_.triangle(primitive, pa, pb, pc, uva, uvb, uvc);

    float2 const duv02       = uva - uvc;
    float2 const duv12       = uvb - uvc;
    float const  determinant = duv02[0] * duv12[1] - duv02[1] * duv12[0];

    float3 dpdu, dpdv;

    float3 const dp02 = pa - pc;
    float3 const dp12 = pb - pc;

    if (std::abs(determinant) == 0.f) {
        float3 const ng = normalize(cross(pc - pa, pb - pa));

        if (std::abs(ng[0]) > std::abs(ng[1])) {
            dpdu = float3(-ng[2], 0, ng[0]) / std::sqrt(ng[0] * ng[0] + ng[2] * ng[2]);
        } else {
            dpdu = float3(0, ng[2], -ng[1]) / std::sqrt(ng[1] * ng[1] + ng[2] * ng[2]);
        }

        dpdv = cross(ng, dpdu);
    } else {
        float const invdet = 1.f / determinant;

        dpdu = invdet * (duv12[1] * dp02 - duv02[1] * dp12);
        dpdv = invdet * (-duv12[0] * dp02 + duv02[0] * dp12);
    }

    return {dpdu, dpdv};
}

void Mesh::prepare_sampling(uint32_t part, bool two_sided, light::Tree_builder& builder,
                            Threads& threads) {
    auto& p = parts_[part];

    // This counts the triangles for _every_ part as an optimization
    if (!primitive_mapping_) {
        primitive_mapping_ = new uint32_t[tree_.num_triangles()];

        for (uint32_t i = 0, len = num_parts(); i < len; ++i) {
            parts_[i].num_triangles = 0;
        }

        for (uint32_t i = 0, len = tree_.num_triangles(); i < len; ++i) {
            uint32_t const pm = parts_[tree_.triangle_part(i)].num_triangles++;

            primitive_mapping_[i] = pm;
        }
    }

    p.init(part, two_sided, tree_, builder, threads);
}

float4 Mesh::cone(uint32_t part) const {
    return parts_[part].cone;
}

}  // namespace scene::shape::triangle
