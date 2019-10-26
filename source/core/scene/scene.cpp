#include "scene.inl"
#include "animation/animation.hpp"
#include "base/math/aabb.inl"
#include "base/math/distribution/distribution_1d.inl"
#include "base/math/matrix3x3.inl"
#include "base/math/quaternion.inl"
#include "base/math/vector3.inl"
#include "base/memory/array.inl"
#include "base/spectrum/rgb.hpp"
#include "bvh/scene_bvh_tree.inl"
#include "entity/composed_transformation.inl"
#include "extension.hpp"
#include "image/texture/texture.hpp"
#include "light/light.inl"
#include "prop/prop.inl"
#include "prop/prop_intersection.hpp"
#include "resource/resource.hpp"
#include "scene_constants.hpp"
#include "scene_ray.hpp"
#include "scene_worker.hpp"
#include "shape/shape.hpp"

#include "base/debug/assert.hpp"

namespace scene {

static size_t constexpr Num_reserved_props = 32;

Scene::Scene(Shape_ptr null_shape, std::vector<Shape*> const& shape_resources,
             std::vector<Material*> const& material_resources,
             std::vector<Texture*> const&  texture_resources) noexcept
    : null_shape_(null_shape),
      shape_resources_(shape_resources),
      material_resources_(material_resources),
      texture_resources_(texture_resources) {
    props_.reserve(Num_reserved_props);
    prop_world_transformations_.reserve(Num_reserved_props);
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
    animation_stages_.reserve(Num_reserved_props);
    finite_props_.reserve(Num_reserved_props);
    infinite_props_.reserve(3);
    volumes_.reserve(Num_reserved_props);
    infinite_volumes_.reserve(1);
}

Scene::~Scene() noexcept {
    clear();
}

void Scene::clear() noexcept {
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

    for (auto a : animations_) {
        delete a;
    }

    animations_.clear();

    animation_stages_.clear();

    for (auto e : extensions_) {
        delete e;
    }

    extensions_.clear();
}

void Scene::finish() noexcept {
    if (lights_.empty()) {
        uint32_t const dummy = create_dummy();
        lights_.emplace_back(light::Light::Type::Null, dummy, prop::Null);
    }

    light_powers_.resize(uint32_t(lights_.size()));
}

AABB const& Scene::aabb() const noexcept {
    return prop_bvh_.aabb();
}

AABB Scene::caustic_aabb() const noexcept {
    AABB aabb = AABB::empty();

    for (auto const i : finite_props_) {
        if (prop_has_caustic_material(i)) {
            aabb.merge_assign(prop_aabbs_[i]);
        }
    }

    return aabb;
}

bool Scene::is_infinite() const noexcept {
    return !infinite_props_.empty() || !infinite_volumes_.empty();
}

bool Scene::has_volumes() const noexcept {
    return has_volumes_;
}

Scene::Light Scene::light(uint32_t id, bool calculate_pdf) const noexcept {
    // If the assert doesn't hold it would pose a problem,
    // but I think it is more efficient to handle those cases outside or implicitely.
    SOFT_ASSERT(!lights_.empty() && light::Light::is_light(id));

    id = light::Light::strip_mask(id);

    float const pdf = calculate_pdf ? light_distribution_.pdf(id) : 1.f;

    return {lights_[id], pdf, id};
}

Scene::Light Scene::random_light(float random) const noexcept {
    SOFT_ASSERT(!lights_.empty());

    auto const l = light_distribution_.sample_discrete(random);

    SOFT_ASSERT(l.offset < uint32_t(lights_.size()));

    return {lights_[l.offset], l.pdf, l.offset};
}

void Scene::simulate(uint64_t start, uint64_t end, thread::Pool& threads) noexcept {
    uint64_t const frames_start = start - (start % tick_duration_);
    uint64_t const end_rem      = end % tick_duration_;
    uint64_t const frames_end   = end + (end_rem ? tick_duration_ - end_rem : 0);

    for (auto a : animations_) {
        a->resample(frames_start, frames_end, tick_duration_);
    }

    for (auto& s : animation_stages_) {
        s.update(*this, threads);
    }

    for (auto m : material_resources_) {
        m->simulate(start, end, tick_duration_, threads, *this);
    }

    compile(start, threads);
}

void Scene::compile(uint64_t time, thread::Pool& threads) noexcept {
    has_masked_material_ = false;
    has_tinted_shadow_   = false;

    for (auto e : extensions_) {
        e->update(*this);
    }

    uint32_t ei = 0;
    for (auto& p : props_) {
        prop_calculate_world_transformation(ei);
        has_masked_material_ = has_masked_material_ || p.has_masked_material();
        has_tinted_shadow_   = has_tinted_shadow_ || p.has_tinted_shadow();

        ++ei;
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
    for (uint32_t i = 0, len = uint32_t(lights_.size()); i < len; ++i) {
        auto& l = lights_[i];
        l.prepare_sampling(i, time, *this, threads);
        light_powers_[i] = std::sqrt(spectrum::luminance(l.power(prop_bvh_.aabb(), *this)));
    }

    light_distribution_.init(light_powers_.data(), uint32_t(light_powers_.size()));

    has_volumes_ = !volumes_.empty() || !infinite_volumes_.empty();
}

void Scene::calculate_num_interpolation_frames(uint64_t frame_step,
                                               uint64_t frame_duration) noexcept {
    num_interpolation_frames_ = count_frames(frame_step, frame_duration) + 1;
}

uint32_t Scene::create_dummy() noexcept {
    auto const prop = allocate_prop();

    prop.ptr->configure(null_shape_, nullptr);

    return prop.id;
}

uint32_t Scene::create_dummy(std::string const& name) noexcept {
    uint32_t const dummy = create_dummy();

    add_named_prop(dummy, name);

    return dummy;
}

uint32_t Scene::create_prop(Shape_ptr shape, Materials const& materials) noexcept {
    auto const prop = allocate_prop();

    prop.ptr->configure(shape, materials.data());

    uint32_t const num_parts = shape.ptr->num_parts();

    // This calls a very simple test to check whether the prop added just before this one
    // has the same shape, same materials, and is not a light.
    if (prop_is_instance(shape, materials, num_parts)) {
        prop_parts_[prop.id] = prop_parts_[props_.size() - 2];
    } else {
        uint32_t const parts_start = uint32_t(materials_.size());

        prop_parts_[prop.id] = parts_start;

        for (uint32_t i = 0; i < num_parts; ++i) {
            materials_.emplace_back(materials[shape.ptr->part_id_to_material_id(i)].id);

            light_ids_.emplace_back(light::Null);
        }
    }

    if (shape.ptr->is_finite()) {
        finite_props_.push_back(prop.id);
    } else {
        infinite_props_.push_back(prop.id);
    }

    // Shape has no surface
    if (1 == num_parts && 1.f == materials[0].ptr->ior()) {
        if (shape.ptr->is_finite()) {
            volumes_.push_back(prop.id);
        } else {
            infinite_volumes_.push_back(prop.id);
        }
    }

    return prop.id;
}

uint32_t Scene::create_prop(Shape_ptr shape, Materials const& materials,
                            std::string const& name) noexcept {
    uint32_t const prop = create_prop(shape, materials);

    add_named_prop(prop, name);

    return prop;
}

void Scene::create_prop_light(uint32_t prop, uint32_t part) noexcept {
    lights_.emplace_back(light::Light::Type::Prop, prop, part);
}

void Scene::create_prop_image_light(uint32_t prop, uint32_t part) noexcept {
    lights_.emplace_back(light::Light::Type::Prop_image, prop, part);
}

void Scene::create_prop_volume_light(uint32_t prop, uint32_t part) noexcept {
    lights_.emplace_back(light::Light::Type::Volume, prop, part);
}

void Scene::create_prop_volume_image_light(uint32_t prop, uint32_t part) noexcept {
    lights_.emplace_back(light::Light::Type::Volume_image, prop, part);
}

uint32_t Scene::create_extension(Extension* extension) noexcept {
    extensions_.push_back(extension);

    uint32_t const dummy = create_dummy();

    extension->init(dummy);

    return dummy;
}

uint32_t Scene::create_extension(Extension* extension, std::string const& name) noexcept {
    extensions_.push_back(extension);

    uint32_t const dummy = create_dummy(name);

    extension->init(dummy);

    return dummy;
}

void Scene::prop_serialize_child(uint32_t parent_id, uint32_t child_id) noexcept {
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

void Scene::prop_set_transformation(uint32_t entity, math::Transformation const& t) noexcept {
    uint32_t const f = prop_frames_[entity];

    entity::Keyframe& local_frame = keyframes_[f + num_interpolation_frames_];

    local_frame.transformation = t;
    local_frame.time           = scene::Static_time;
}

Scene::Transformation const& Scene::prop_transformation_at(uint32_t entity, uint64_t time,
                                                           Transformation& transformation) const
    noexcept {
    uint32_t const f = prop_frames_[entity];

    if (prop::Null == f) {
        return prop_world_transformation(entity);
    }

    entity::Keyframe const* frames = &keyframes_[f];

    for (uint32_t i = 0, len = num_interpolation_frames_ - 1; i < len; ++i) {
        auto const& a = frames[i];
        auto const& b = frames[i + 1];

        if (time >= a.time && time < b.time) {
            uint64_t const range = b.time - a.time;
            uint64_t const delta = time - a.time;

            float const t = float(delta) / float(range);

            transformation.set(lerp(a.transformation, b.transformation, t));

            break;
        }
    }

    return transformation;
}

void Scene::prop_allocate_frames(uint32_t entity, bool local_animation) noexcept {
    prop_frames_[entity] = uint32_t(keyframes_.size());

    uint32_t const num_world_frames = num_interpolation_frames_;
    uint32_t const num_local_frames = local_animation ? num_world_frames : 1;

    uint32_t const num_frames = num_world_frames + num_local_frames;

    for (uint32_t i = 0; i < num_frames; ++i) {
        keyframes_.emplace_back();
    }

    props_[entity].configure_animated(entity, local_animation, *this);
}

bool Scene::prop_has_animated_frames(uint32_t entity) const noexcept {
    return prop::Null != prop_frames_[entity];
}

void Scene::prop_set_frames(uint32_t entity, animation::Keyframe const* frames) noexcept {
    uint32_t const num_frames = num_interpolation_frames_;

    uint32_t const f = prop_frames_[entity];

    entity::Keyframe* local_frames = &keyframes_[f + num_frames];
    for (uint32_t i = 0; i < num_frames; ++i) {
        local_frames[i] = frames[i].k;
    }
}

void Scene::prop_calculate_world_transformation(uint32_t entity) noexcept {
    auto const& p = props_[entity];

    if (p.has_no_parent()) {
        uint32_t const f = prop_frames_[entity];

        if (prop::Null != f) {
            entity::Keyframe* frames = &keyframes_[f];
            for (uint32_t i = 0, len = num_interpolation_frames_; i < len; ++i) {
                frames[i] = frames[len + i];
            }
        }

        prop_propagate_transformation(entity);
    }
}

void Scene::prop_propagate_transformation(uint32_t entity) noexcept {
    uint32_t const f = prop_frames_[entity];

    if (prop::Null == f) {
        auto const& transformation = prop_world_transformation(entity);

        prop_aabbs_[entity] = prop_shape(entity)->transformed_aabb(
            transformation.object_to_world());

        for (uint32_t child = prop_topology(entity).child; prop::Null != child;) {
            prop_inherit_transformation(child, transformation);

            child = prop_topology(child).next;
        }
    } else {
        static uint32_t constexpr Num_steps = 4;

        static float constexpr Interval = 1.f / float(Num_steps);

        entity::Keyframe const* frames = &keyframes_[f];

        Shape const* shape = prop_shape(entity);

        AABB aabb = shape->transformed_aabb(float4x4(frames[0].transformation));

        for (uint32_t i = 0, len = num_interpolation_frames_ - 1; i < len; ++i) {
            auto const& a = frames[i].transformation;
            auto const& b = frames[i + 1].transformation;

            float t = Interval;
            for (uint32_t j = Num_steps - 1; j > 0; --j, t += Interval) {
                math::Transformation const interpolated = lerp(a, b, t);

                aabb.merge_assign(shape->transformed_aabb(float4x4(interpolated)));
            }

            prop_aabbs_[entity] = aabb.merge(shape->transformed_aabb(float4x4(b)));
        }

        for (uint32_t child = prop_topology(entity).child; prop::Null != child;) {
            prop_inherit_transformation(child, frames);

            child = prop_topology(child).next;
        }
    }
}

void Scene::prop_inherit_transformation(uint32_t              entity,
                                        Transformation const& transformation) noexcept {
    uint32_t const f = prop_frames_[entity];

    if (prop::Null != f) {
        entity::Keyframe* frames = &keyframes_[f];

        bool const local_animation = prop(entity)->has_local_animation();

        for (uint32_t i = 0, len = num_interpolation_frames_; i < len; ++i) {
            uint32_t const lf = local_animation ? i : 0;
            frames[len + lf].transform(frames[i], transformation);
        }
    }

    prop_propagate_transformation(entity);
}

void Scene::prop_inherit_transformation(uint32_t entity, entity::Keyframe const* frames) noexcept {
    bool const local_animation = prop(entity)->has_local_animation();

    entity::Keyframe* tf = &keyframes_[prop_frames_[entity]];

    for (uint32_t i = 0, len = num_interpolation_frames_; i < len; ++i) {
        uint32_t const lf = local_animation ? i : 0;
        tf[len + lf].transform(tf[i], frames[i]);
    }

    prop_propagate_transformation(entity);
}

void Scene::prop_set_visibility(uint32_t entity, bool in_camera, bool in_reflection,
                                bool in_shadow) noexcept {
    props_[entity].set_visibility(in_camera, in_reflection, in_shadow);
}

void Scene::prop_prepare_sampling(uint32_t entity, uint32_t part, uint32_t light_id, uint64_t time,
                                  bool          material_importance_sampling,
                                  thread::Pool& threads) noexcept {
    auto shape = prop_shape(entity);

    shape->prepare_sampling(part);

    Transformation temp;
    auto const&    transformation = prop_transformation_at(entity, time, temp);

    float const area = shape->area(part, transformation.scale());

    lights_[light_id].set_area(area);

    uint32_t const p = prop_parts_[entity] + part;

    light_ids_[p] = light_id;

    material_resources_[materials_[p]]->prepare_sampling(
        *shape, part, time, transformation, area, material_importance_sampling, threads, *this);
}

void Scene::prop_prepare_sampling_volume(uint32_t entity, uint32_t part, uint32_t light_id,
                                         uint64_t time, bool material_importance_sampling,
                                         thread::Pool& threads) noexcept {
    auto shape = prop_shape(entity);

    shape->prepare_sampling(part);

    Transformation temp;
    auto const&    transformation = prop_transformation_at(entity, time, temp);

    float const volume = shape->volume(part, transformation.scale());

    lights_[light_id].set_volume(volume);

    uint32_t const p = prop_parts_[entity] + part;

    light_ids_[p] = light_id;

    material_resources_[materials_[p]]->prepare_sampling(
        *shape, part, time, transformation, volume, material_importance_sampling, threads, *this);
}

animation::Animation* Scene::create_animation(uint32_t count) noexcept {
    animation::Animation* animation = new animation::Animation(count, num_interpolation_frames_);

    animations_.push_back(animation);

    return animation;
}

void Scene::create_animation_stage(uint32_t entity, animation::Animation* animation) noexcept {
    animation_stages_.emplace_back(entity, animation);
    animation_stages_.back().allocate_enitity_frames(*this);
}

size_t Scene::num_bytes() const noexcept {
    size_t num_bytes = 0;

    for (auto& p : props_) {
        num_bytes += p.num_bytes();
    }

    return num_bytes + sizeof(*this);
}

void Scene::prop_animated_transformation_at(uint32_t entity, uint64_t time,
                                            Transformation& transformation) const noexcept {
    entity::Keyframe const* frames = &keyframes_[prop_frames_[entity]];

    for (uint32_t i = 0, len = num_interpolation_frames_ - 1; i < len; ++i) {
        auto const& a = frames[i];
        auto const& b = frames[i + 1];

        if (time >= a.time && time < b.time) {
            uint64_t const range = b.time - a.time;
            uint64_t const delta = time - a.time;

            float const t = float(delta) / float(range);

            transformation.set(lerp(a.transformation, b.transformation, t));

            break;
        }
    }
}

Scene::Prop_ptr Scene::allocate_prop() noexcept {
    props_.emplace_back();
    prop_world_transformations_.emplace_back();
    prop_parts_.emplace_back();
    prop_frames_.emplace_back(prop::Null);
    prop_topology_.emplace_back();
    prop_aabbs_.emplace_back();

    uint32_t const prop_id = uint32_t(props_.size()) - 1;

    prop::Prop* prop = &props_[prop_id];

    return {prop, prop_id};
}

bool Scene::prop_is_instance(Shape_ptr shape, Materials const& materials, uint32_t num_parts) const
    noexcept {
    if (props_.size() < 2 || props_[props_.size() - 2].shape() != shape.id) {
        return false;
    }

    uint32_t const p = prop_parts_[props_.size() - 2];
    for (uint32_t i = 0; i < num_parts; ++i) {
        if (materials[shape.ptr->part_id_to_material_id(i)].id != materials_[p + i]) {
            return false;
        }

        if (light_ids_[p + i] != light::Null) {
            return false;
        }
    }

    return true;
}

bool Scene::prop_has_caustic_material(uint32_t entity) const noexcept {
    auto const shape = prop_shape(entity);
    for (uint32_t i = 0, len = shape->num_parts(); i < len; ++i) {
        if (prop_material(entity, i)->is_caustic()) {
            return true;
        }
    }

    return false;
}

void Scene::add_named_prop(uint32_t prop, std::string const& name) noexcept {
    if (!prop || name.empty()) {
        return;
    }

    if (named_props_.find(name) != named_props_.end()) {
        return;
    }

    named_props_.insert_or_assign(name, prop);
}

static inline bool matching(uint64_t a, uint64_t b) noexcept {
    return 0 == (a > b ? a % b : (0 == a ? 0 : b % a));
}

uint32_t Scene::count_frames(uint64_t frame_step, uint64_t frame_duration) const noexcept {
    uint32_t const a = std::max(uint32_t(frame_duration / tick_duration_), 1u);
    uint32_t const b = matching(frame_step, tick_duration_) ? 0 : 1;
    uint32_t const c = matching(frame_duration, tick_duration_) ? 0 : 1;

    return a + b + c;
}

}  // namespace scene
