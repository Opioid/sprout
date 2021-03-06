#include "scene.inl"
#include "animation/animation.hpp"
#include "base/math/aabb.inl"
#include "base/math/cone.inl"
#include "base/math/distribution_1d.inl"
#include "base/math/matrix3x3.inl"
#include "base/math/quaternion.inl"
#include "base/math/vector3.inl"
#include "base/memory/array.inl"
#include "base/spectrum/rgb.hpp"
#include "bvh/scene_bvh_tree.inl"
#include "composed_transformation.inl"
#include "extension.hpp"
#include "light/light.inl"
#include "light/light_tree_builder.hpp"
#include "material/material.inl"
#include "prop/prop.inl"
#include "scene_ray.hpp"
#include "scene_worker.hpp"
#include "shape/shape.inl"

#include "base/debug/assert.hpp"

// #define DISABLE_LIGHT_TREE 1

namespace scene {

static size_t constexpr Num_reserved_props = 32;

static uint32_t constexpr Num_steps = 4;

static float constexpr Interval = 1.f / float(Num_steps);

static uint32_t count_frames(uint64_t frame_step, uint64_t frame_duration);

Scene::Scene(std::vector<Image*> const&    image_resources,
             std::vector<Material*> const& material_resources,
             std::vector<Shape*> const& shape_resources, uint32_t null_shape)
    : image_resources_(image_resources),
      material_resources_(material_resources),
      shape_resources_(shape_resources),
      null_shape_(null_shape) {
    props_.reserve(Num_reserved_props);
    prop_world_transformations_.reserve(Num_reserved_props);
    prop_world_positions_.reserve(Num_reserved_props);
    prop_parts_.reserve(Num_reserved_props);
    prop_frames_.reserve(Num_reserved_props);
    prop_topology_.reserve(Num_reserved_props);
    prop_aabbs_.reserve(Num_reserved_props);
    materials_.reserve(Num_reserved_props);
    light_ids_.reserve(Num_reserved_props);
    keyframes_.reserve(Num_reserved_props);
    lights_.reserve(Num_reserved_props);
    extensions_.reserve(Num_reserved_props);
    animations_.reserve(Num_reserved_props);
    finite_props_.reserve(Num_reserved_props);
    infinite_props_.reserve(3);
    volumes_.reserve(Num_reserved_props);
}

Scene::~Scene() {
    clear();
}

void Scene::clear() {
    prop_parts_.clear();
    prop_topology_.clear();
    prop_frames_.clear();
    prop_aabbs_.clear();
    prop_world_transformations_.clear();

    lights_.clear();

    keyframes_.clear();
    light_ids_.clear();
    materials_.clear();

    infinite_volumes_.clear();
    volumes_.clear();

    infinite_props_.clear();
    finite_props_.clear();
    props_.clear();
    animations_.clear();

    for (auto e : extensions_) {
        delete e;
    }

    extensions_.clear();
}

Light_pick Scene::light(uint32_t id, float3_p p, float3_p n, bool total_sphere, bool split) const {
#ifdef DISABLE_LIGHT_TREE

    id = light::Light::strip_mask(id);

    float const pdf = light_distribution_.pdf(id);

    return {light(id), id, pdf};

#else

    SOFT_ASSERT(!lights_.empty() && light::Light::is_light(id));

    id = light::Light::strip_mask(id);

    float const pdf = light_tree_.pdf(p, n, total_sphere, split, id, *this);

    return {light(id), id, pdf};

#endif
}

Light_pick Scene::light(uint32_t id, float3_p p0, float3_p p1, bool split) const {
#ifdef DISABLE_LIGHT_TREE

    id = light::Light::strip_mask(id);

    float const pdf = light_distribution_.pdf(id);

    return {light(id), id, pdf};

#else

    id = light::Light::strip_mask(id);

    float const pdf = light_tree_.pdf(p0, p1, split, id, *this);

    return {light(id), id, pdf};

#endif
}

Light_pick Scene::random_light(float random) const {
    SOFT_ASSERT(!lights_.empty());

    auto const l = light_distribution_.sample_discrete(random);

    SOFT_ASSERT(l.offset < uint32_t(lights_.size()));

    return {light(l.offset), l.offset, l.pdf};
}

void Scene::random_light(float3_p p, float3_p n, bool total_sphere, float random, bool split,
                         Lights& lights) const {
#ifdef DISABLE_LIGHT_TREE

    auto const l = light_distribution_.sample_discrete(random);

    lights.clear();
    lights.push_back(l);

#else

    light_tree_.random_light(p, n, total_sphere, random, split, *this, lights);

#ifdef SU_DEBUG

    for (auto const l : lights) {
        float const guessed_pdf = light_tree_.pdf(p, n, total_sphere, split, l.offset, *this);

        float const diff = std::abs(guessed_pdf - l.pdf);

        SOFT_ASSERT(diff < 1e-8f);
    }

#endif

#endif
}

void Scene::random_light(float3_p p0, float3_p p1, float random, bool split, Lights& lights) const {
#ifdef DISABLE_LIGHT_TREE

    auto const l = light_distribution_.sample_discrete(random);

    lights.clear();
    lights.push_back(l);

#else

    light_tree_.random_light(p0, p1, random, split, *this, lights);

#ifdef SU_DEBUG

    for (auto const l : lights) {
        float const guessed_pdf = light_tree_.pdf(p0, p1, split, l.offset, *this);

        float const diff = std::abs(guessed_pdf - l.pdf);

        SOFT_ASSERT(diff < 1e-8f);
    }

#endif

#endif
}

void Scene::simulate(float3_p camera_pos, uint64_t start, uint64_t end, Worker& worker,
                     Threads& threads) {
    uint64_t const frames_start = start - (start % Tick_duration);
    uint64_t const end_rem      = end % Tick_duration;
    uint64_t const frames_end   = end + (end_rem ? Tick_duration - end_rem : 0);

    current_time_start_ = frames_start;

    for (auto& a : animations_) {
        a.resample(frames_start, frames_end, Tick_duration);
        a.update(*this, threads);
    }

    compile(camera_pos, start, worker, threads);
}

void Scene::compile(float3_p camera_pos, uint64_t time, Worker& worker, Threads& threads) {
    has_tinted_shadow_ = false;

    for (auto e : extensions_) {
        e->update(*this);
    }

    for (uint32_t i = 0; auto& p : props_) {
        prop_calculate_world_transformation(i, camera_pos);
        has_tinted_shadow_ = has_tinted_shadow_ || p.has_tinted_shadow();

        ++i;
    }

    for (auto v : volumes_) {
        props_[v].set_visible_in_shadow(false);
    }

    // rebuild prop BVH
    bvh_builder_.build(prop_bvh_.tree(), finite_props_, prop_aabbs_, threads);
    prop_bvh_.set_props(infinite_props_, props_);

    // rebuild volume BVH
    bvh_builder_.build(volume_bvh_.tree(), volumes_, prop_aabbs_, threads);
    volume_bvh_.set_props(infinite_volumes_, props_);

    // re-sort lights PDF
    light_temp_powers_.resize(uint32_t(lights_.size()));

    for (uint32_t i = 0; auto& l : lights_) {
        l.prepare_sampling(i, time, *this, worker, threads);
        light_temp_powers_[i] = light_power(0, i);

        ++i;
    }

    light_distribution_.init(light_temp_powers_.data(), light_temp_powers_.size());

    light_tree_builder_.build(light_tree_, *this, threads);

    has_volumes_ = !volumes_.empty() || !infinite_volumes_.empty();

    AABB caustic_aabb = Empty_AABB;

    for (auto const i : finite_props_) {
        if (prop_has_caustic_material(i)) {
            caustic_aabb.merge_assign(prop_aabbs_[i]);
        }
    }

    caustic_aabb_ = caustic_aabb;
}

void Scene::commit_materials(Threads& threads) const {
    for (auto m : material_resources_) {
        m->commit(threads, *this);
    }
}

void Scene::calculate_num_interpolation_frames(uint64_t frame_step, uint64_t frame_duration) {
    num_interpolation_frames_ = count_frames(frame_step, frame_duration) + 1;
}

uint32_t Scene::create_entity() {
    auto const prop = allocate_prop();

    prop.ptr->configure(null_shape_, nullptr, *this);

    return prop.id;
}

uint32_t Scene::create_prop(uint32_t shape, uint32_t const* materials) {
    auto const prop = allocate_prop();

    prop.ptr->configure(shape, materials, *this);

    Shape const* shape_ptr = Scene::shape(shape);

    uint32_t const num_parts = shape_ptr->num_parts();

    // This calls a very simple test to check whether the prop added just before this one
    // has the same shape, same materials, and is not a light.
    if (prop_is_instance(shape, materials, num_parts)) {
        prop_parts_[prop.id] = prop_parts_[props_.size() - 2];
    } else {
        uint32_t const parts_start = uint32_t(materials_.size());

        prop_parts_[prop.id] = parts_start;

        for (uint32_t i = 0; i < num_parts; ++i) {
            materials_.emplace_back(materials[shape_ptr->part_id_to_material_id(i)]);

            light_ids_.emplace_back(light::Null);
        }
    }

    if (shape_ptr->is_finite()) {
        finite_props_.push_back(prop.id);
    } else {
        infinite_props_.push_back(prop.id);
    }

    // Shape has no surface
    if (1 == num_parts && 1.f == material(materials[0])->ior()) {
        if (shape_ptr->is_finite()) {
            volumes_.push_back(prop.id);
        } else {
            infinite_volumes_.push_back(prop.id);
        }
    }

    return prop.id;
}

void Scene::create_light(uint32_t prop) {
    auto const shape = prop_shape(prop);

    for (uint32_t i = 0, len = shape->num_parts(); i < len; ++i) {
        if (auto const material = prop_material(prop, i); material->is_emissive()) {
            if (material->is_scattering_volume()) {
                if (shape->is_analytical() && material->has_emission_map()) {
                    allocate_light(light::Light::Type::Volume_image, false, prop, i);
                } else {
                    allocate_light(light::Light::Type::Volume, false, prop, i);
                }
            } else {
                bool const two_sided = material->is_two_sided();

                if (shape->is_analytical() && material->has_emission_map()) {
                    allocate_light(light::Light::Type::Prop_image, two_sided, prop, i);
                } else {
                    allocate_light(light::Light::Type::Prop, two_sided, prop, i);
                }
            }
        }
    }
}

uint32_t Scene::create_extension(Extension* extension) {
    extensions_.push_back(extension);

    uint32_t const dummy = create_entity();

    extension->init(dummy);

    return dummy;
}

void Scene::prop_serialize_child(uint32_t parent_id, uint32_t child_id) {
    props_[child_id].set_has_parent();

    if (prop_has_animated_frames(parent_id) && !prop_has_animated_frames(child_id)) {
        // This is the case if child has no animation attached to it directly
        prop_allocate_frames(child_id, false);
    }

    prop::Prop_topology& pt = prop_topology_[parent_id];

    if (prop::Null == pt.child) {
        pt.child = child_id;
    } else {
        prop_topology_[prop_topology_.size() - 2].next = child_id;
    }
}

void Scene::prop_allocate_frames(uint32_t entity, bool local_animation) {
    prop_frames_[entity] = uint32_t(keyframes_.size());

    uint32_t const num_world_frames = num_interpolation_frames_;
    uint32_t const num_local_frames = local_animation ? num_world_frames : 1;

    uint32_t const num_frames = num_world_frames + num_local_frames;

    for (uint32_t i = 0; i < num_frames; ++i) {
        keyframes_.emplace_back();
    }

    props_[entity].configure_animated(local_animation, *this);
}

bool Scene::prop_has_animated_frames(uint32_t entity) const {
    return prop::Null != prop_frames_[entity];
}

void Scene::prop_set_frames(uint32_t entity, math::Transformation const* frames) {
    uint32_t const num_frames = num_interpolation_frames_;

    uint32_t const f = prop_frames_[entity];

    math::Transformation* local_frames = &keyframes_[f + num_frames];
    for (uint32_t i = 0; i < num_frames; ++i) {
        local_frames[i] = frames[i];
    }
}

void Scene::prop_set_frame(uint32_t entity, uint32_t frame, math::Transformation const& k) {
    uint32_t const num_frames = num_interpolation_frames_;

    uint32_t const f = prop_frames_[entity];

    math::Transformation* local_frames = &keyframes_[f + num_frames];

    local_frames[frame] = k;
}

void Scene::prop_set_visibility(uint32_t entity, bool in_camera, bool in_reflection,
                                bool in_shadow) {
    props_[entity].set_visibility(in_camera, in_reflection, in_shadow);
}

void Scene::prop_prepare_sampling(uint32_t entity, uint32_t part, uint32_t light, uint64_t time,
                                  bool volume, Worker& worker, Threads& threads) {
    auto shape = prop_shape(entity);

    uint32_t const p = prop_parts_[entity] + part;

    uint32_t const m = materials_[p];

    uint32_t const variant = shape->prepare_sampling(part, m, light_tree_builder_, worker, threads);

    lights_[light].set_variant(variant);

    Transformation temp;
    auto const&    trafo = prop_transformation_at(entity, time, temp);

    float3 const scale = trafo.scale();

    float const extent = volume ? shape->volume(part, scale) : shape->area(part, scale);

    light_ids_[p] = volume ? (light::Light::Volume_light_mask | light) : light;

    Material& material = *material_resources_[m];

    float3 const average_radiance = material.prepare_sampling(*shape, part, trafo, extent, *this,
                                                              threads);

    lights_[light].set_extent(extent);

    uint32_t const f = prop_frames_[entity];

    AABB const part_aabb = shape->part_aabb(part, variant);

    if (prop::Null == f) {
        AABB bb = part_aabb.transform(trafo.object_to_world());
        bb.cache_radius();

        light_aabbs_[light] = bb;

        float4 const cone = shape->cone(part);

        light_cones_[light] = float4(trafo.object_to_world_normal(cone.xyz()), cone[3]);
    } else {
        math::Transformation const* frames = &keyframes_[f];

        AABB bb = part_aabb.transform(float4x4(frames[0]));

        float4 const part_cone = shape->cone(part);

        float4 cone = float4(trafo.object_to_world_normal(part_cone.xyz()), part_cone[3]);

        for (uint32_t i = 0, len = num_interpolation_frames_ - 1; i < len; ++i) {
            auto const& a = frames[i];
            auto const& b = frames[i + 1];

            float t = Interval;
            for (uint32_t j = Num_steps - 1; j > 0; --j, t += Interval) {
                math::Transformation const inter = lerp(a, b, t);

                float3x3 const rotation = quaternion::create_matrix3x3(inter.rotation);
                float4x4 const composed = compose(rotation, inter.scale, inter.position);

                bb.merge_assign(part_aabb.transform(composed));
                cone = cone::merge(cone, cone::transform(rotation, cone));
            }

            float3x3 const rotation = quaternion::create_matrix3x3(b.rotation);
            float4x4 const composed = compose(rotation, b.scale, b.position);

            bb.merge_assign(part_aabb.transform(composed));
            cone = cone::merge(cone, cone::transform(rotation, cone));
        }

        bb.cache_radius();

        light_aabbs_[light] = bb;
        light_cones_[light] = cone;
    }

    light_aabbs_[light].bounds[1][3] = max_component(
        lights_[light].power(average_radiance, aabb(), *this));
}

uint32_t Scene::create_animation(uint32_t entity, uint32_t count) {
    animations_.emplace_back(entity, count, num_interpolation_frames_);

    prop_allocate_frames(entity, true);

    return uint32_t(animations_.size() - 1);
}

void Scene::animation_set_frame(uint32_t animation, uint32_t index,
                                animation::Keyframe const& keyframe) {
    animations_[animation].set(index, keyframe);
}

void Scene::prop_calculate_world_transformation(uint32_t entity, float3_p camera_pos) {
    auto const& p = props_[entity];

    if (p.has_no_parent()) {
        uint32_t const f = prop_frames_[entity];

        if (prop::Null != f) {
            math::Transformation* frames = &keyframes_[f];
            for (uint32_t i = 0, len = num_interpolation_frames_; i < len; ++i) {
                frames[i].set(frames[len + i], camera_pos);
            }
        }

        prop_propagate_transformation(entity, camera_pos);
    }
}

void Scene::prop_propagate_transformation(uint32_t entity, float3_p camera_pos) {
    uint32_t const f = prop_frames_[entity];

    AABB const shape_aabb = prop_shape(entity)->aabb();

    if (prop::Null == f) {
        auto& trafo = prop_world_transformations_[entity];

        trafo.set_position(prop_world_positions_[entity] - camera_pos);

        prop_aabbs_[entity] = shape_aabb.transform(trafo.object_to_world());

        for (uint32_t child = prop_topology(entity).child; prop::Null != child;) {
            prop_inherit_transformation(child, trafo, camera_pos);

            child = prop_topology(child).next;
        }
    } else {
        math::Transformation const* frames = &keyframes_[f];

        AABB aabb = shape_aabb.transform(float4x4(frames[0]));

        for (uint32_t i = 0, len = num_interpolation_frames_ - 1; i < len; ++i) {
            auto const& a = frames[i];
            auto const& b = frames[i + 1];

            float t = Interval;
            for (uint32_t j = Num_steps - 1; j > 0; --j, t += Interval) {
                math::Transformation const inter = lerp(a, b, t);

                aabb.merge_assign(shape_aabb.transform(float4x4(inter)));
            }

            aabb.merge_assign(shape_aabb.transform(float4x4(b)));
        }

        prop_aabbs_[entity] = aabb;

        for (uint32_t child = prop_topology(entity).child; prop::Null != child;) {
            prop_inherit_transformation(child, frames, camera_pos);

            child = prop_topology(child).next;
        }
    }
}

void Scene::prop_inherit_transformation(uint32_t entity, Transformation const& trafo,
                                        float3_p camera_pos) {
    uint32_t const f = prop_frames_[entity];

    if (prop::Null != f) {
        math::Transformation* frames = &keyframes_[f];

        bool const local_animation = prop(entity)->has_local_animation();

        for (uint32_t i = 0, len = num_interpolation_frames_; i < len; ++i) {
            uint32_t const lf = local_animation ? i : 0;
            frames[i]         = transform(frames[len + lf], trafo);
        }
    }

    prop_propagate_transformation(entity, camera_pos);
}

void Scene::prop_inherit_transformation(uint32_t entity, math::Transformation const* frames,
                                        float3_p camera_pos) {
    bool const local_animation = prop(entity)->has_local_animation();

    math::Transformation* tf = &keyframes_[prop_frames_[entity]];

    for (uint32_t i = 0, len = num_interpolation_frames_; i < len; ++i) {
        uint32_t const lf = local_animation ? i : 0;
        tf[i]             = transform(tf[len + lf], frames[i]);
    }

    prop_propagate_transformation(entity, camera_pos);
}

Scene::Transformation const& Scene::prop_animated_transformation_at(uint32_t        frames_id,
                                                                    uint64_t        time,
                                                                    Transformation& trafo) const {
    auto const f = frame_at(time);

    math::Transformation const* frames = &keyframes_[frames_id];

    auto const& a = frames[f.f];
    auto const& b = frames[f.f + 1];

    trafo.set(lerp(a, b, f.w));

    return trafo;
}

Scene::Prop_ptr Scene::allocate_prop() {
    props_.emplace_back();
    prop_world_transformations_.emplace_back();
    prop_world_positions_.emplace_back();
    prop_parts_.emplace_back();
    prop_frames_.emplace_back(prop::Null);
    prop_topology_.emplace_back();
    prop_aabbs_.emplace_back();

    uint32_t const prop_id = uint32_t(props_.size() - 1);

    prop::Prop* prop = &props_[prop_id];

    return {prop, prop_id};
}

void Scene::allocate_light(light::Light::Type type, bool two_sided, uint32_t entity,
                           uint32_t part) {
    lights_.emplace_back(type, two_sided, entity, part);

    light_aabbs_.emplace_back(AABB(float3(0.f), float3(0.f)));
    light_cones_.emplace_back(float4(0.f, 0.f, 1.f, -1.f));
}

bool Scene::prop_is_instance(uint32_t shape, uint32_t const* materials, uint32_t num_parts) const {
    if (props_.size() < 2 || props_[props_.size() - 2].shape() != shape) {
        return false;
    }

    Shape const* shape_ptr = shape_resources_[shape];

    uint32_t const p = prop_parts_[props_.size() - 2];
    for (uint32_t i = 0; i < num_parts; ++i) {
        uint32_t const m = materials[shape_ptr->part_id_to_material_id(i)];

        if (m != materials_[p + i]) {
            return false;
        }

        if (material_resources_[m]->is_emissive()) {
            return false;
        }
    }

    return true;
}

bool Scene::prop_has_caustic_material(uint32_t entity) const {
    auto const shape = prop_shape(entity);
    for (uint32_t i = 0, len = shape->num_parts(); i < len; ++i) {
        if (prop_material(entity, i)->is_caustic()) {
            return true;
        }
    }

    return false;
}

static inline bool matching(uint64_t a, uint64_t b) {
    return 0 == (a > b ? a % b : (0 == a ? 0 : b % a));
}

uint32_t count_frames(uint64_t frame_step, uint64_t frame_duration) {
    uint32_t const a = std::max(uint32_t(frame_duration / Tick_duration), 1u);
    uint32_t const b = matching(frame_step, Tick_duration) ? 0 : 1;
    uint32_t const c = matching(frame_duration, Tick_duration) ? 0 : 1;

    return a + b + c;
}

}  // namespace scene
