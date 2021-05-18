#include "triangle_mesh.hpp"
#include "base/math/aabb.inl"
#include "base/math/distribution_1d.inl"
#include "base/math/matrix3x3.inl"
#include "base/math/matrix4x4.inl"
#include "base/math/sample_distribution.inl"
#include "base/math/sampling.inl"
#include "base/math/vector3.inl"
#include "base/memory/array.inl"
#include "base/memory/buffer.hpp"
#include "base/thread/thread_pool.hpp"
#include "bvh/triangle_bvh_tree.inl"
#include "sampler/sampler.hpp"
#include "scene/entity/composed_transformation.inl"
#include "scene/light/light.hpp"
#include "scene/light/light_tree_builder.hpp"
#include "scene/material/material.inl"
#include "scene/ray_offset.inl"
#include "scene/scene_ray.inl"
#include "scene/shape/shape_intersection.hpp"
#include "scene/shape/shape_sample.hpp"

#ifdef SU_DEBUG
#include "scene/shape/shape_test.hpp"
#endif
#include "base/debug/assert.hpp"

namespace scene::shape::triangle {

Part::Variant::Variant() : cones(nullptr) {}

Part::Variant::Variant(Variant&& other)
    : cones(other.cones),
      distribution(std::move(other.distribution)),
      light_tree(std::move(other.light_tree)),
      cone(other.cone),
      material(other.material),
      two_sided_(other.two_sided_) {
    other.cones = nullptr;
}

Part::Variant::~Variant() {
    delete[] cones;
}

bool Part::Variant::matches(uint32_t m, bool emission_map, bool two_sided,
                            Scene const& scene) const {
    if (material == m) {
        return true;
    }

    auto const& lm = scene.material(material);

    if (!lm->has_emission_map() && !emission_map) {
        return two_sided_ == two_sided;
    }

    return false;
}

Part::~Part() {
    delete[] aabbs_;
    delete[] triangle_mapping_;
}

static float triangle_area(float2 a, float2 b, float2 c) {
    float2 const x = b - a;
    float2 const y = c - a;

    return 0.5f * std::abs(x[0] * y[1] - x[1] * y[0]);
}

uint32_t Part::init(uint32_t part, uint32_t material, bvh::Tree const& tree,
                    light::Tree_builder& builder, Worker const& worker, Threads& threads) {
    using Filter = material::Sampler_settings::Filter;

    uint32_t const num = num_triangles;

    if (!triangle_mapping_) {
        triangle_mapping_ = new uint32_t[num];

        aabbs_ = new AABB[num];

        float total_area = 0.f;

        for (uint32_t t = 0, mt = 0, len = tree.num_triangles(); t < len; ++t) {
            if (tree.triangle_part(t) == part) {
                float const area = tree.triangle_area(t);

                total_area += area;

                triangle_mapping_[mt] = t;

                float3 va;
                float3 vb;
                float3 vc;
                tree.triangle(t, va, vb, vc);

                AABB box(Empty_AABB);
                box.insert(va);
                box.insert(vb);
                box.insert(vc);
                box.cache_radius();
                box.bounds[1][3] = area;

                aabbs_[mt] = box;

                ++mt;
            }
        }

        area_ = total_area;

        for (uint32_t i = 0; i < num; ++i) {
            aabbs_[i].bounds[1][3] = total_area / aabbs_[i].bounds[1][3];
        }
    }

    Material const& m = *worker.scene().material(material);

    bool const emission_map = m.has_emission_map();
    bool const two_sided    = m.is_two_sided();

    for (uint32_t v = 0; auto const& variant : variants_) {
        if (variant.matches(material, emission_map, two_sided, worker.scene())) {
            return v;
        }

        ++v;
    }

    uint32_t const v = uint32_t(variants_.size());

    variants_.emplace_back();

    Variant& variant = variants_[v];

    variant.cones = new float4[num];

    memory::Buffer<float> powers(num);

    struct Temp {
        AABB   bb            = Empty_AABB;
        float3 dominant_axis = float3(0.f);
        float  total_power   = 0.f;
    };

    memory::Array<Temp> temps(threads.num_threads());

    int2 const dimensions = m.useful_texture_description(worker.scene()).dimensions().xy();

    float const estimate_area = float(dimensions[0] * dimensions[1]) / 4.f;

    static float3 constexpr Up = float3(0.f, 1.f, 0.f);

    threads.run_range(
        [this, &variant, &tree, &m, &powers, &temps, &worker, estimate_area](
            uint32_t id, int32_t begin, int32_t end) noexcept {
            bool const emission_map = m.has_emission_map();

            Temp temp;

            for (int32_t i = begin; i < end; ++i) {
                uint32_t const t = triangle_mapping_[i];

                float const area = tree.triangle_area(t);

                float power;
                if (emission_map) {
                    float3 va;
                    float3 vb;
                    float3 vc;
                    float2 uva;
                    float2 uvb;
                    float2 uvc;
                    tree.triangle(t, va, vb, vc, uva, uvb, uvc);

                    float const uv_area = triangle_area(uva, uvb, uvc);

                    uint32_t const num_samples = std::max(std::lrint(uv_area * estimate_area + 0.5f), 1l);

                    float3 radiance(0.f);

                    for (uint32_t j = 0; j < num_samples; ++j) {
                        float2 const xi = hammersley(j, num_samples, 0);
                        float2 const s2 = sample_triangle_uniform(xi);
                        float2 const uv = tree.interpolate_triangle_uv(Simd3f(s2[0]), Simd3f(s2[1]),
                                                                       t);

                        radiance += m.evaluate_radiance(Up, Up, float3(uv), 1.f, Filter::Undefined,
                                                        worker);
                    }

                    float const weight = max_component(radiance) / float(num_samples);

                    power = weight * area;
                } else {
                    power = area;
                }

                powers[i] = power;

                float3 const n = tree.triangle_normal(t);

                variant.cones[i] = float4(n, 1.f);

                if (power > 0.f) {
                    temp.dominant_axis += power * n;

                    temp.bb.merge_assign(aabbs_[i]);

                    temp.total_power += power;
                }
            }

            temps[id] = temp;
        },
        0, int32_t(num));

    variant.distribution.init(powers, num);

    Temp temp;
    for (auto const& t : temps) {
        temp.bb.merge_assign(t.bb);

        temp.dominant_axis += t.dominant_axis;

        temp.total_power += t.total_power;
    }

    temp.dominant_axis = normalize(temp.dominant_axis / temp.total_power);

    float angle = 0.f;

    for (uint32_t i = 0; i < num; ++i) {
        uint32_t const t = triangle_mapping_[i];

        float3 const n = tree.triangle_normal(t);
        float const  c = dot(temp.dominant_axis, n);

        SOFT_ASSERT(std::isfinite(c));

        angle = std::max(angle, std::acos(c));
    }

    variant.aabb = temp.bb;
    variant.cone = float4(temp.dominant_axis, std::cos(angle));

    variant.material   = material;
    variant.two_sided_ = two_sided;

    builder.build(variant.light_tree, *this, v, threads);

    return v;
}

light::Pick Part::sample(uint32_t variant, float3_p p, float3_p n, bool total_sphere,
                         float r) const {
    auto const pick = variants_[variant].light_tree.random_light(p, n, total_sphere, r, *this,
                                                                 variant);

    float const relative_area = aabbs_[pick.offset].bounds[1][3];

    return {triangle_mapping_[pick.offset], pick.pdf * relative_area};
}

float Part::pdf(uint32_t variant, float3_p p, float3_p n, bool total_sphere, uint32_t id) const {
    float const pdf = variants_[variant].light_tree.pdf(p, n, total_sphere, id, *this, variant);

    float const relative_area = aabbs_[id].bounds[1][3];

    return pdf * relative_area;
}

math::Distribution_1D::Discrete Part::sample(uint32_t variant, float r) const {
    auto const result = variants_[variant].distribution.sample_discrete(r);

    float const relative_area = aabbs_[result.offset].bounds[1][3];

    return {triangle_mapping_[result.offset], result.pdf * relative_area};
}

AABB const& Part::aabb(uint32_t variant) const {
    return variants_[variant].aabb;
}

float Part::power(uint32_t variant) const {
    return variants_[variant].distribution.integral();
}

float4_p Part::cone(uint32_t variant) const {
    return variants_[variant].cone;
}

AABB const& Part::light_aabb(uint32_t light) const {
    SOFT_ASSERT(light < num_triangles);

    return aabbs_[light];
}

float4_p Part::light_cone(uint32_t variant, uint32_t light) const {
    SOFT_ASSERT(light < num_triangles);

    return variants_[variant].cones[light];
}

bool Part::light_two_sided(uint32_t variant, uint32_t /*light*/) const {
    return variants_[variant].two_sided_;
}

float Part::light_power(uint32_t variant, uint32_t light) const {
    SOFT_ASSERT(light < num_triangles);

    auto const& v = variants_[variant];

    return v.distribution.pdf(light) * v.distribution.integral();
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
    parts_[part].material_ = material;
}

AABB Mesh::aabb() const {
    return tree_.aabb();
}

AABB Mesh::part_aabb(uint32_t part, uint32_t variant) const {
    return parts_[part].aabb(variant);
}

uint32_t Mesh::num_parts() const {
    return tree_.num_parts();
}

uint32_t Mesh::num_materials() const {
    uint32_t id = 0;

    for (uint32_t i = 0, len = num_parts(); i < len; ++i) {
        id = std::max(id, parts_[i].material_);
    }

    return id + 1;
}

uint32_t Mesh::part_id_to_material_id(uint32_t part) const {
    return parts_[part].material_;
}

bool Mesh::intersect(Ray& ray, Transformation const& trafo, Node_stack& nodes, Interpolation ipo,
                     shape::Intersection& isec) const {
    Simd4x4f const world_to_object(trafo.world_to_object);

    Simd3f const ray_origin    = transform_point(world_to_object, Simd3f(ray.origin));
    Simd3f const ray_direction = transform_vector(world_to_object, Simd3f(ray.direction));

    scalar const ray_min_t(ray.min_t());
    scalar       ray_max_t(ray.max_t());

    if (Intersection pi;
        tree_.intersect(ray_origin, ray_direction, ray_min_t, ray_max_t, nodes, pi)) {
        ray.max_t() = ray_max_t.x();

        Simd3f const p = tree_.interpolate_p(pi.u, pi.v, pi.index);

        Simd4x4f const object_to_world(trafo.object_to_world());

        Simd3f const p_w = transform_point(object_to_world, p);

        Simd3f const geo_n = tree_.triangle_normal_v(pi.index);

        Simd3x3f const rotation(trafo.rotation);

        Simd3f const geo_n_w = transform_vector(rotation, geo_n);

        isec.p         = float3(p_w);
        isec.geo_n     = float3(geo_n_w);
        isec.part      = tree_.triangle_part(pi.index);
        isec.primitive = pi.index;

        if (Interpolation::All == ipo) {
            Simd3f n;
            Simd3f t;
            float2 uv;
            tree_.interpolate_triangle_data(pi.u, pi.v, pi.index, n, t, uv);

            Simd3f const bitangent_sign(tree_.triangle_bitangent_sign(pi.index));

            Simd3f const n_w = transform_vector(rotation, n);
            Simd3f const t_w = transform_vector(rotation, t);
            Simd3f const b_w = bitangent_sign * cross(n_w, t_w);

            isec.t  = float3(t_w);
            isec.b  = float3(b_w);
            isec.n  = float3(n_w);
            isec.uv = uv;
        } else if (Interpolation::No_tangent_space == ipo) {
            float2 const uv = tree_.interpolate_triangle_uv(pi.u, pi.v, pi.index);

            isec.uv = uv;
        } else {
            Simd3f const n   = tree_.interpolate_shading_normal(pi.u, pi.v, pi.index);
            Simd3f const n_w = transform_vector(rotation, n);

            isec.n = float3(n_w);
        }

        SOFT_ASSERT(testing::check(isec, trafo, ray));

        return true;
    }

    return false;
}

bool Mesh::intersect_p(Ray const& ray, Transformation const& trafo, Node_stack& nodes) const {
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

bool Mesh::sample(uint32_t part, uint32_t variant, float3_p p, float3_p n,
                  Transformation const& trafo, float area, bool two_sided, bool total_sphere,
                  Sampler& sampler, RNG& rng, uint32_t sampler_d, Sample_to& sample) const {
    float const  r  = sampler.sample_1D(rng, sampler_d);
    float2 const r2 = sampler.sample_2D(rng, sampler_d);

    float3 const op = trafo.world_to_object_point(p);
    float3 const on = trafo.world_to_object_normal(n);
    auto const   s  = parts_[part].sample(variant, op, on, total_sphere, r);

    if (0.f == s.pdf) {
        return false;
    }

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

    float const guessed_pdf = parts_[part].pdf(variant, op, on, total_sphere, pm);

    float const diff = std::abs(guessed_pdf - s.pdf);

    SOFT_ASSERT(diff < 1e-8f);

    if (diff >= 1e-8f) {
        std::cout << "problem " << s.pdf << "  " << guessed_pdf << std::endl;
    }
#endif

    return true;
}

bool Mesh::sample(uint32_t part, uint32_t variant, Transformation const& trafo, float area,
                  bool /*two_sided*/, Sampler& sampler, RNG& rng, uint32_t sampler_d,
                  float2 importance_uv, AABB const& /*bounds*/, Sample_from& sample) const {
    float const r = sampler.sample_1D(rng, sampler_d);
    auto const  s = parts_[part].sample(variant, r);

    float2 const r0 = sampler.sample_2D(rng, sampler_d);

    float3 sv;
    float2 tc;
    tree_.sample(s.offset, r0, sv, tc);
    float3 const ws = trafo.object_to_world_point(sv);

    float3 const sn = tree_.triangle_normal(s.offset);
    float3 const wn = transform_vector(trafo.rotation, sn);

    auto const [x, y] = orthonormal_basis(wn);

    float3 const dir = sample_oriented_hemisphere_cosine(importance_uv, x, y, wn);

    sample = Sample_from(offset_ray(ws, wn), wn, dir, tc, importance_uv, s.pdf / (Pi * area));

    return true;
}

float Mesh::pdf(uint32_t variant, Ray const& ray, float3_p n, shape::Intersection const& isec,
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

    float const tri_pdf = parts_[isec.part].pdf(variant, op, on, total_sphere, pm);

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

bool Mesh::sample_volume(uint32_t /*part*/, float3_p /*p*/, float3_p /*uvw*/,
                         Transformation const& /*trafo*/, float /*volume*/,
                         Sample_to& /*sample*/) const {
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
    return parts_[part].area_ * (scale[0] * scale[1]);
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

uint32_t Mesh::prepare_sampling(uint32_t part, uint32_t material, light::Tree_builder& builder,
                                Worker const& worker, Threads& threads) {
    // This counts the triangles for _every_ part as an optimization
    if (!primitive_mapping_) {
        primitive_mapping_ = new uint32_t[tree_.num_triangles()];

        for (uint32_t i = 0, len = tree_.num_triangles(); i < len; ++i) {
            uint32_t const pm = parts_[tree_.triangle_part(i)].num_triangles++;

            primitive_mapping_[i] = pm;
        }
    }

    return parts_[part].init(part, material, tree_, builder, worker, threads);
}

float4 Mesh::cone(uint32_t part) const {
    return parts_[part].variants_[0].cone;
}

}  // namespace scene::shape::triangle
