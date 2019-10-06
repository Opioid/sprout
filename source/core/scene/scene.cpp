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
#include "prop/prop.hpp"
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
             std::vector<Material*> const& material_resources) noexcept
    : null_shape_(null_shape),
      shape_resources_(shape_resources),
      material_resources_(material_resources) {
    props_.reserve(Num_reserved_props);
    prop_world_transformations_.reserve(Num_reserved_props);
    prop_materials_.reserve(Num_reserved_props);
    prop_frames_.reserve(Num_reserved_props);
    prop_topology_.reserve(Num_reserved_props);
    prop_aabbs_.reserve(Num_reserved_props);
    finite_props_.reserve(Num_reserved_props);
    infinite_props_.reserve(3);
    volumes_.reserve(Num_reserved_props);
    infinite_volumes_.reserve(1);
    lights_.reserve(Num_reserved_props);
    extensions_.reserve(Num_reserved_props);
    materials_.reserve(Num_reserved_props);
    animations_.reserve(Num_reserved_props);
    animation_stages_.reserve(Num_reserved_props);
}

Scene::~Scene() noexcept {
    clear();
}

void Scene::clear() noexcept {
    for (auto& m : prop_materials_) {
        memory::free_aligned(m.parts);
        memory::free_aligned(m.materials);
    }

    prop_materials_.clear();
    prop_topology_.clear();
    prop_frames_.clear();
    prop_aabbs_.clear();
    prop_morphing_.clear();
    prop_world_transformations_.clear();

    lights_.clear();

    infinite_volumes_.clear();
    volumes_.clear();

    infinite_props_.clear();
    finite_props_.clear();
    props_.clear();

    materials_.clear();

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
        prop_allocate_frames(dummy, 1, 1);
        lights_.emplace_back(light::Light::Type::Null, dummy, prop::Null);
    }

    light_powers_.resize(lights_.size());
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

bool Scene::intersect(Ray& ray, Worker const& worker, prop::Intersection& intersection) const
    noexcept {
    return prop_bvh_.intersect(ray, worker, intersection);
}

bool Scene::intersect(Ray& ray, Worker const& worker, shape::Normals& normals) const noexcept {
    return prop_bvh_.intersect(ray, worker, normals);
}

bool Scene::intersect_volume(Ray& ray, Worker const& worker, prop::Intersection& intersection) const
    noexcept {
    return volume_bvh_.intersect_fast(ray, worker, intersection);
}

bool Scene::intersect_p(Ray const& ray, Worker const& worker) const noexcept {
    return prop_bvh_.intersect_p(ray, worker);
}

bool Scene::visibility(Ray const& ray, Filter filter, Worker const& worker, float& v) const
    noexcept {
    if (has_masked_material_) {
        return prop_bvh_.visibility(ray, filter, worker, v);
    }

    if (!prop_bvh_.intersect_p(ray, worker)) {
        v = 1.f;
        return true;
    }

    v = 0.f;
    return false;
}

bool Scene::thin_absorption(Ray const& ray, Filter filter, Worker const& worker, float3& ta) const
    noexcept {
    if (has_tinted_shadow_) {
        return prop_bvh_.thin_absorption(ray, filter, worker, ta);
    }

    if (float v; Scene::visibility(ray, filter, worker, v)) {
        ta = float3(v);
        return true;
    }

    return false;
}

std::vector<light::Light> const& Scene::lights() const noexcept {
    return lights_;
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

void Scene::simulate(uint64_t start, uint64_t end, thread::Pool& thread_pool) noexcept {
    uint64_t const frames_start = start - (start % tick_duration_);
    uint64_t const end_rem      = end % tick_duration_;
    uint64_t const frames_end   = end + (end_rem ? tick_duration_ - end_rem : 0);

    for (auto a : animations_) {
        a->resample(frames_start, frames_end, tick_duration_);
    }

    for (auto& s : animation_stages_) {
        s.update(*this);
    }

    uint32_t i = 0;
    for (auto& p : props_) {
        p.morph(i, thread_pool, *this);
        ++i;
    }

    for (auto m : materials_) {
        material_resources_[m]->simulate(start, end, tick_duration_, thread_pool);
    }

    compile(start, thread_pool);
}

void Scene::compile(uint64_t time, thread::Pool& pool) noexcept {
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
    bvh_builder_.build(prop_bvh_.tree(), finite_props_, prop_aabbs_);
    prop_bvh_.set_props(finite_props_, infinite_props_, props_);

    // rebuild volume BVH
    bvh_builder_.build(volume_bvh_.tree(), volumes_, prop_aabbs_);
    volume_bvh_.set_props(volumes_, infinite_volumes_, props_);

    // re-sort lights PDF
    for (uint32_t i = 0, len = uint32_t(lights_.size()); i < len; ++i) {
        auto& l = lights_[i];
        l.prepare_sampling(i, time, *this, pool);
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

    auto& m = prop_materials_[prop.id];

    m.parts     = memory::allocate_aligned<prop::Prop_material::Part>(num_parts);
    m.materials = memory::allocate_aligned<uint32_t>(num_parts);

    for (uint32_t i = 0; i < num_parts; ++i) {
        auto& p = m.parts[i];

        p.area     = 1.f;
        p.light_id = 0xFFFFFFFF;

        m.materials[i] = materials[shape.ptr->part_id_to_material_id(i)].id;
    }

    if (shape.ptr->is_finite()) {
        finite_props_.push_back(prop.id);
    } else {
        infinite_props_.push_back(prop.id);
    }

    // Shape has no surface
    if (1 == shape.ptr->num_parts() && 1.f == materials[0].ptr->ior()) {
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
    prop::Prop_topology& pt = prop_topology_[parent_id];

    props_[child_id].set_has_parent();

    if (0 == prop_frames_[child_id].num_local_frames) {
        // This is the case if n has no animation attached to it directly
        prop_allocate_frames(child_id, prop_frames_[parent_id].num_world_frames, 1);
    }

    if (prop::Null == pt.child) {
        pt.child = child_id;
    } else {
        prop_topology_[prop_topology_.size() - 2].next = child_id;
    }
}

void Scene::prop_set_transformation(uint32_t entity, math::Transformation const& t) noexcept {
    prop::Prop_frames const& f = prop_frames_[entity];

    entity::Keyframe& local_frame = f.frames[f.num_world_frames];

    local_frame.transformation = t;
    local_frame.time           = scene::Static_time;
}

Scene::Transformation const& Scene::prop_transformation_at(uint32_t entity, uint64_t time,
                                                           Transformation& transformation) const
    noexcept {
    prop::Prop_frames const& f = prop_frames_[entity];

    if (1 == f.num_world_frames) {
        return prop_world_transformation(entity);
    }

    for (uint32_t i = 0, len = f.num_world_frames - 1; i < len; ++i) {
        auto const& a = f.frames[i];
        auto const& b = f.frames[i + 1];

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

void Scene::prop_allocate_frames(uint32_t entity, uint32_t num_world_frames,
                                 uint32_t num_local_frames) noexcept {
    prop_frames_[entity].num_world_frames = num_world_frames;
    prop_frames_[entity].num_local_frames = num_local_frames;

    prop_frames_[entity].frames = memory::allocate_aligned<entity::Keyframe>(num_world_frames +
                                                                             num_local_frames);

    props_[entity].allocate_frames(entity, num_world_frames, *this);
}

void Scene::prop_set_frames(uint32_t entity, animation::Keyframe const* frames,
                            uint32_t num_frames) noexcept {
    prop::Prop_frames const& f = prop_frames_[entity];

    entity::Keyframe* local_frames = &f.frames[f.num_world_frames];
    for (uint32_t i = 0; i < num_frames; ++i) {
        local_frames[i] = frames[i].k;
    }

    prop_set_morphing(entity, frames[0].m);
}

void Scene::prop_calculate_world_transformation(uint32_t entity) noexcept {
    if (props_[entity].has_no_children()) {
        prop::Prop_frames const& f = prop_frames_[entity];
        for (uint32_t i = 0, len = f.num_world_frames; i < len; ++i) {
            f.frames[i] = f.frames[len + i];
        }

        prop_propagate_transformation(entity);
    }
}

void Scene::prop_propagate_transformation(uint32_t entity) noexcept {
    prop::Prop_frames const& f = prop_frames_[entity];

    Shape const* shape = prop_shape(entity);

    // Prop::on_set_transformation()
    if (1 == f.num_world_frames) {
        prop_set_world_transformation(entity, f.frames[0].transformation);

        auto const& t = prop_world_transformation(entity);

        prop_aabbs_[entity] = shape->transformed_aabb(t.object_to_world,
                                                      f.frames[0].transformation);
    } else {
        static uint32_t constexpr Num_steps = 4;

        static float constexpr Interval = 1.f / float(Num_steps);

        AABB aabb = shape->transformed_aabb(f.frames[0].transformation);

        for (uint32_t i = 0, len = f.num_world_frames - 1; i < len; ++i) {
            auto const& a = f.frames[i].transformation;
            auto const& b = f.frames[i + 1].transformation;

            float t = Interval;
            for (uint32_t j = Num_steps - 1; j > 0; --j, t += Interval) {
                math::Transformation const interpolated = lerp(a, b, t);

                aabb.merge_assign(shape->transformed_aabb(interpolated));
            }

            prop_aabbs_[entity] = aabb.merge(shape->transformed_aabb(b));
        }
    }

    // ---

    entity::Keyframe const* frames = f.frames;

    uint32_t const num_world_frames = f.num_world_frames;

    for (uint32_t child = prop_topology(entity).child; prop::Null != child;) {
        prop_inherit_transformation(child, frames, num_world_frames);

        child = prop_topology(child).next;
    }
}

void Scene::prop_inherit_transformation(uint32_t entity, entity::Keyframe const* frames,
                                        uint32_t num_frames) noexcept {
    prop::Prop_frames const& f = prop_frames_[entity];

    uint32_t const num_local_frames = f.num_local_frames;
    for (uint32_t i = 0, len = f.num_world_frames; i < len; ++i) {
        uint32_t const lf = num_local_frames > 1 ? i : 0;
        uint32_t const of = num_frames > 1 ? i : 0;
        f.frames[len + lf].transform(f.frames[i], frames[of]);
    }

    prop_propagate_transformation(entity);
}

entity::Morphing const& Scene::prop_morphing(uint32_t entity) const noexcept {
    return prop_morphing_[entity];
}

void Scene::prop_set_morphing(uint32_t entity, entity::Morphing const& morphing) noexcept {
    prop_morphing_[entity] = morphing;
}

void Scene::prop_set_visibility(uint32_t entity, bool in_camera, bool in_reflection,
                                bool in_shadow) noexcept {
    props_[entity].set_visibility(in_camera, in_reflection, in_shadow);
}

void Scene::prop_prepare_sampling(uint32_t entity, uint32_t part, uint32_t light_id, uint64_t time,
                                  bool material_importance_sampling, thread::Pool& pool) noexcept {
    auto shape = prop_shape(entity);

    shape->prepare_sampling(part);

    Transformation temp;
    auto const&    transformation = prop_transformation_at(entity, time, temp);

    float const area = shape->area(part, transformation.scale);

    auto& m = prop_materials_[entity];

    m.parts[part].area = area;

    m.parts[part].light_id = light_id;

    material_resources_[m.materials[part]]->prepare_sampling(
        *shape, part, time, transformation, area, material_importance_sampling, pool);
}

void Scene::prop_prepare_sampling_volume(uint32_t entity, uint32_t part, uint32_t light_id,
                                         uint64_t time, bool material_importance_sampling,
                                         thread::Pool& pool) noexcept {
    auto shape = prop_shape(entity);

    shape->prepare_sampling(part);

    Transformation temp;
    auto const&    transformation = prop_transformation_at(entity, time, temp);

    float const volume = shape->volume(part, transformation.scale);

    auto& m = prop_materials_[entity];

    m.parts[part].volume = volume;

    m.parts[part].light_id = light_id;

    material_resources_[m.materials[part]]->prepare_sampling(
        *shape, part, time, transformation, volume, material_importance_sampling, pool);
}

void Scene::add_material(uint32_t material) noexcept {
    materials_.push_back(material);
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

Scene::Prop_ptr Scene::allocate_prop() noexcept {
    props_.emplace_back();
    prop_world_transformations_.emplace_back();
    prop_morphing_.emplace_back();
    prop_materials_.emplace_back();
    prop_frames_.emplace_back();
    prop_topology_.emplace_back();
    prop_aabbs_.emplace_back();

    uint32_t const prop_id = uint32_t(props_.size()) - 1;

    prop::Prop* prop = &props_[prop_id];

    return {prop, prop_id};
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
