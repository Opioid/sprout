#include "triangle_mesh.hpp"
#include "base/math/distribution/distribution_1d.inl"
#include "base/math/matrix3x3.inl"
#include "base/math/matrix4x4.inl"
#include "base/math/sampling.inl"
#include "base/math/vector3.inl"
#include "base/memory/buffer.hpp"
#include "bvh/triangle_bvh_tree.inl"
#include "sampler/sampler.hpp"
#include "scene/entity/composed_transformation.inl"
#include "scene/scene_constants.hpp"
#include "scene/scene_ray.inl"
#include "scene/shape/shape_intersection.hpp"
#include "scene/shape/shape_sample.hpp"
#include "triangle_intersection.hpp"

#ifdef SU_DEBUG
#include "scene/shape/shape_test.hpp"
#endif
#include "base/debug/assert.hpp"

namespace scene::shape::triangle {

Mesh::Mesh()
    : Shape(Properties(Property::Complex, Property::Finite)),
      distributions_(nullptr),
      part_materials_(nullptr) {}

Mesh::~Mesh() {
    delete[] part_materials_;
    delete[] distributions_;
}

Tree& Mesh::tree() {
    return tree_;
}

void Mesh::allocate_parts(uint32_t num_parts) {
    tree_.allocate_parts(num_parts);

    distributions_ = new Distribution[num_parts];

    part_materials_ = new uint32_t[num_parts];
}

void Mesh::set_material_for_part(uint32_t part, uint32_t material) {
    part_materials_[part] = material;
}

float3 Mesh::object_to_texture_point(float3 const& p) const {
    return (p - tree_.aabb().bounds[0]) / tree_.aabb().extent();
}

float3 Mesh::object_to_texture_vector(float3 const& v) const {
    return v / tree_.aabb().extent();
}

AABB Mesh::transformed_aabb(float4x4 const& m) const {
    return tree_.aabb().transform(m);
}

uint32_t Mesh::num_parts() const {
    return tree_.num_parts();
}

uint32_t Mesh::num_materials() const {
    uint32_t id = 0;

    for (uint32_t i = 0, len = num_parts(); i < len; ++i) {
        id = std::max(id, part_materials_[i]);
    }

    return id + 1;
}

uint32_t Mesh::part_id_to_material_id(uint32_t part) const {
    return part_materials_[part];
}

bool Mesh::intersect(Ray& ray, Transformation const& transformation, Node_stack& node_stack,
                     shape::Intersection& intersection) const {
    Simd4x4f const world_to_object(transformation.world_to_object);

    Simd3f const ray_origin    = transform_point(world_to_object, Simd3f(ray.origin));
    Simd3f const ray_direction = transform_vector(world_to_object, Simd3f(ray.direction));

    scalar const ray_min_t(ray.min_t());
    scalar       ray_max_t(ray.max_t());

    if (Intersection pi;
        tree_.intersect(ray_origin, ray_direction, ray_min_t, ray_max_t, node_stack, pi)) {
        ray.max_t() = ray_max_t.x();

        Simd3f p = tree_.interpolate_p(pi.u, pi.v, pi.index);

        Simd4x4f const object_to_world(transformation.object_to_world());

        Simd3f p_w = transform_point(object_to_world, p);

        Simd3f n;
        Simd3f t;
        float2 uv;
        tree_.interpolate_triangle_data(pi.u, pi.v, pi.index, n, t, uv);

        Simd3f geo_n = tree_.triangle_normal_v(pi.index);

        Simd3f bitangent_sign(tree_.triangle_bitangent_sign(pi.index));

        uint32_t part = tree_.triangle_part(pi.index);

        Simd3x3f rotation(transformation.rotation);

        Simd3f geo_n_w = transform_vector(rotation, geo_n);
        Simd3f n_w     = transform_vector(rotation, n);
        Simd3f t_w     = transform_vector(rotation, t);
        Simd3f b_w     = bitangent_sign * cross(n_w, t_w);

        intersection.p         = float3(p_w);
        intersection.t         = float3(t_w);
        intersection.b         = float3(b_w);
        intersection.n         = float3(n_w);
        intersection.geo_n     = float3(geo_n_w);
        intersection.uv        = uv;
        intersection.part      = part;
        intersection.primitive = pi.index;

        SOFT_ASSERT(testing::check(intersection, transformation, ray));

        return true;
    }

    return false;
}

bool Mesh::intersect_nsf(Ray& ray, Transformation const& transformation, Node_stack& node_stack,
                         shape::Intersection& intersection) const {
    Simd4x4f const world_to_object(transformation.world_to_object);

    Simd3f const ray_origin    = transform_point(world_to_object, Simd3f(ray.origin));
    Simd3f const ray_direction = transform_vector(world_to_object, Simd3f(ray.direction));

    scalar const ray_min_t(ray.min_t());
    scalar       ray_max_t(ray.max_t());

    if (Intersection pi;
        tree_.intersect(ray_origin, ray_direction, ray_min_t, ray_max_t, node_stack, pi)) {
        ray.max_t() = ray_max_t.x();

        Simd3f p = tree_.interpolate_p(pi.u, pi.v, pi.index);

        Simd4x4f const object_to_world(transformation.object_to_world());

        Simd3f p_w = transform_point(object_to_world, p);

        float2 const uv = tree_.interpolate_triangle_uv(pi.u, pi.v, pi.index);

        Simd3f geo_n = tree_.triangle_normal_v(pi.index);

        uint32_t part = tree_.triangle_part(pi.index);

        Simd3x3f rotation(transformation.rotation);

        Simd3f geo_n_w = transform_vector(rotation, geo_n);

        intersection.p         = float3(p_w);
        intersection.geo_n     = float3(geo_n_w);
        intersection.uv        = uv;
        intersection.part      = part;
        intersection.primitive = pi.index;

        return true;
    }

    return false;
}

bool Mesh::intersect(Ray& ray, Transformation const& transformation, Node_stack& node_stack,
                     Normals& normals) const {
    Simd4x4f const world_to_object(transformation.world_to_object);

    Simd3f const ray_origin    = transform_point(world_to_object, Simd3f(ray.origin));
    Simd3f const ray_direction = transform_vector(world_to_object, Simd3f(ray.direction));

    scalar const ray_min_t(ray.min_t());
    scalar       ray_max_t(ray.max_t());

    if (Intersection pi;
        tree_.intersect(ray_origin, ray_direction, ray_min_t, ray_max_t, node_stack, pi)) {
        ray.max_t() = ray_max_t.x();

        Simd3f n = tree_.interpolate_shading_normal(pi.u, pi.v, pi.index);

        Simd3f geo_n = tree_.triangle_normal_v(pi.index);

        Simd3x3f rotation(transformation.rotation);

        Simd3f geo_n_w = transform_vector(rotation, geo_n);
        Simd3f n_w     = transform_vector(rotation, n);

        normals.geo_n = float3(geo_n_w);
        normals.n     = float3(n_w);

        return true;
    }

    return false;
}

bool Mesh::intersect_p(Ray const& ray, Transformation const& transformation,
                       Node_stack& node_stack) const {
    //	ray tray;
    //	tray.origin = transform_point(ray.origin, transformation.world_to_object);
    //	tray.set_direction(transform_vector(ray.direction, transformation.world_to_object));
    //	tray.min_t = ray.min_t;
    //	tray.max_t = ray.max_t;

    //	return tree_.intersect_p(tray, node_stack);

    Simd4x4f const world_to_object(transformation.world_to_object);

    Simd3f const ray_origin    = transform_point(world_to_object, Simd3f(ray.origin));
    Simd3f const ray_direction = transform_vector(world_to_object, Simd3f(ray.direction));

    scalar const ray_min_t(ray.min_t());
    scalar const ray_max_t(ray.max_t());

    return tree_.intersect_p(ray_origin, ray_direction, ray_min_t, ray_max_t, node_stack);
}

float Mesh::visibility(Ray const& ray, Transformation const& transformation, uint32_t entity,
                       Filter filter, Worker& worker) const {
    math::ray tray(transformation.world_to_object_point(ray.origin),
                   transformation.world_to_object_vector(ray.direction), ray.min_t(), ray.max_t());

    return tree_.visibility(tray, ray.time, entity, filter, worker);
}

bool Mesh::thin_absorption(Ray const& ray, Transformation const& transformation, uint32_t entity,
                           Filter filter, Worker& worker, float3& ta) const {
    math::ray tray(transformation.world_to_object_point(ray.origin),
                   transformation.world_to_object_vector(ray.direction), ray.min_t(), ray.max_t());

    return tree_.absorption(tray, ray.time, entity, filter, worker, ta);
}

bool Mesh::sample(uint32_t part, float3 const& p, Transformation const& transformation, float area,
                  bool two_sided, Sampler& sampler, rnd::Generator& rng, uint32_t sampler_dimension,
                  Sample_to& sample) const {
    float const  r  = sampler.generate_sample_1D(rng, sampler_dimension);
    float2 const r2 = sampler.generate_sample_2D(rng, sampler_dimension);
    auto const   s  = distributions_[part].sample(r);

    float3 sv;
    float2 tc;
    tree_.sample(s.offset, r2, sv, tc);
    float3 const v = transformation.object_to_world_point(sv);

    float3 const sn = tree_.triangle_normal(s.offset);
    float3 const wn = transform_vector(transformation.rotation, sn);

    float3 const axis = v - p;
    float const  sl   = squared_length(axis);
    float const  d    = std::sqrt(sl);
    float3 const dir  = axis / d;

    float c = -dot(wn, dir);

    if (two_sided) {
        c = std::abs(c);
    }

    sample = Sample_to(dir, float3(tc), sl / (c * area), offset_b(d));

    return true;
}

bool Mesh::sample(uint32_t part, Transformation const& transformation, float area,
                  bool /*two_sided*/, Sampler& sampler, rnd::Generator& rng,
                  uint32_t sampler_dimension, float2 importance_uv, AABB const& /*bounds*/,
                  Sample_from& sample) const {
    float const r = sampler.generate_sample_1D(rng, sampler_dimension);
    auto const  s = distributions_[part].sample(r);

    float2 const r0 = sampler.generate_sample_2D(rng, sampler_dimension);

    float3 sv;
    float2 tc;
    tree_.sample(s.offset, r0, sv, tc);
    float3 const ws = transformation.object_to_world_point(sv);

    float3 const sn = tree_.triangle_normal(s.offset);
    float3 const wn = transform_vector(transformation.rotation, sn);

    auto const [x, y] = orthonormal_basis(wn);

    float3 const dir = sample_oriented_hemisphere_cosine(importance_uv, x, y, wn);

    sample.p   = ws;
    sample.dir = dir;
    sample.xy  = importance_uv;
    sample.pdf = 1.f / ((1.f * Pi) * area);

    return true;
}

float Mesh::pdf(Ray const& ray, shape::Intersection const&      intersection,
                Transformation const& /*transformation*/, float area, bool two_sided,
                bool /*total_sphere*/) const {
    float c = -dot(intersection.geo_n, ray.direction);

    if (two_sided) {
        c = std::abs(c);
    }

    float const sl = ray.max_t() * ray.max_t();
    return sl / (c * area);
}

float Mesh::pdf_volume(Ray const& /*ray*/, shape::Intersection const& /*intersection*/,
                       Transformation const& /*transformation*/, float /*area*/) const {
    return 0.f;
}

bool Mesh::sample(uint32_t /*part*/, float3 const& /*p*/, float2 /*uv*/,
                  Transformation const& /*transformation*/, float /*area*/, bool /*two_sided*/,
                  Sample_to& /*sample*/) const {
    return false;
}

bool Mesh::sample(uint32_t /*part*/, float3 const& /*p*/, float3 const& /*uvw*/,
                  Transformation const& /*transformation*/, float /*volume*/,
                  Sample_to& /*sample*/) const {
    return false;
}

bool Mesh::sample(uint32_t /*part*/, float2 /*uv*/, Transformation const& /*transformation*/,
                  float /*area*/, bool /*two_sided*/, float2 /*importance_uv*/,
                  AABB const& /*bounds*/, Sample_from& /*sample*/) const {
    return false;
}

float Mesh::pdf_uv(Ray const& /*ray*/, shape::Intersection const& /*intersection*/,
                   Transformation const& /*transformation*/, float /*area*/,
                   bool /*two_sided*/) const {
    return 0.f;
}

float Mesh::uv_weight(float2 /*uv*/) const {
    return 1.f;
}

float Mesh::area(uint32_t part, float3 const& scale) const {
    // HACK: This only really works for uniform scales!
    return distributions_[part].distribution.integral() * (scale[0] * scale[1]);
}

float Mesh::volume(uint32_t /*part*/, float3 const& /*scale*/) const {
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

    //  return {float3(1.f, 0.f, 0.f), float3(0.f, -1.f, 0.f)};
}

void Mesh::prepare_sampling(uint32_t part) {
    if (distributions_[part].empty()) {
        auto& d = distributions_[part];

        d.init(part, tree_);

        float3 center(0.f);

        float const n = 1.f / float(d.num_triangles);

        for (uint32_t i = 0, len = d.num_triangles; i < len; ++i) {
            uint32_t const t = d.triangle_mapping[i];

            center += n * tree_.triangle_center(t);
        }

        d.center = center;
    }
}

float3 Mesh::center(uint32_t part) const {
    return distributions_[part].center;
}

Mesh::Distribution::~Distribution() {
    delete[] triangle_mapping;
}

void Mesh::Distribution::init(uint32_t part, Tree const& tree) {
    uint32_t const num = tree.num_triangles(part);

    memory::Buffer<float> areas(num);

    num_triangles = num;

    triangle_mapping = new uint32_t[num];

    for (uint32_t t = 0, mt = 0, len = tree.num_triangles(); t < len; ++t) {
        if (tree.triangle_part(t) == part) {
            areas[mt] = tree.triangle_area(t);

            triangle_mapping[mt] = t;

            ++mt;
        }
    }

    distribution.init(areas, num_triangles);
}

bool Mesh::Distribution::empty() const {
    return nullptr == triangle_mapping;
}

Mesh::Distribution::Distribution_1D::Discrete Mesh::Distribution::sample(float r) const {
    auto const result = distribution.sample_discrete(r);
    return {triangle_mapping[result.offset], result.pdf};
}

}  // namespace scene::shape::triangle
