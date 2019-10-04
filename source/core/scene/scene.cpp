#include "scene.hpp"
#include "animation/animation.hpp"
#include "base/math/aabb.inl"
#include "base/math/distribution/distribution_1d.inl"
#include "base/math/matrix3x3.inl"
#include "base/math/quaternion.inl"
#include "base/math/vector3.inl"
#include "base/memory/array.inl"
#include "base/spectrum/rgb.hpp"
#include "bvh/scene_bvh_builder.inl"
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

Scene::Scene(std::vector<Material*>& material_backup) noexcept
    : material_resources_(material_backup) {
    props_.reserve(Num_reserved_props);
    prop_world_transformations_.reserve(Num_reserved_props);
    prop_materials_.reserve(Num_reserved_props);
    prop_topology_.reserve(Num_reserved_props);
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
            Prop const& p = props_[i];
            aabb.merge_assign(p.aabb());
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

prop::Prop const* Scene::prop(uint32_t index) const noexcept {
    SOFT_ASSERT(index < props_.size());

    return &props_[index];
}

prop::Prop* Scene::prop(uint32_t index) noexcept {
    SOFT_ASSERT(index < props_.size());

    return &props_[index];
}

prop::Prop* Scene::prop(std::string_view name) noexcept {
    auto e = named_props_.find(name);
    if (named_props_.end() == e) {
        return nullptr;
    }

    return &props_[e->second];
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
        p.calculate_world_transformation(ei, *this);
        has_masked_material_ = has_masked_material_ || p.has_masked_material();
        has_tinted_shadow_   = has_tinted_shadow_ || p.has_tinted_shadow();

        ++ei;
    }

    for (auto v : volumes_) {
        props_[v].set_visible_in_shadow(false);
    }

    // rebuild prop BVH
    bvh_builder_.build(prop_bvh_.tree(), finite_props_, props_);
    prop_bvh_.set_props(finite_props_, infinite_props_, props_);

    // rebuild volume BVH
    bvh_builder_.build(volume_bvh_.tree(), volumes_, props_);
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

    prop.ptr->configure(&null_shape_, nullptr);

    return prop.id;
}

uint32_t Scene::create_dummy(std::string const& name) noexcept {
    uint32_t const dummy = create_dummy();

    add_named_prop(dummy, name);

    return dummy;
}

uint32_t Scene::create_prop(Shape* shape, Materials const& materials) noexcept {
    auto const prop = allocate_prop();

    prop.ptr->configure(shape, materials.data());

    uint32_t const num_parts = shape->num_parts();

    auto& m = prop_materials_[prop.id];

    m.parts     = memory::allocate_aligned<prop::Prop_material::Part>(num_parts);
    m.materials = memory::allocate_aligned<uint32_t>(num_parts);

    for (uint32_t i = 0; i < num_parts; ++i) {
        auto& p = m.parts[i];

        p.area     = 1.f;
        p.light_id = 0xFFFFFFFF;

        m.materials[i] = materials[shape->part_id_to_material_id(i)].id;
    }

    if (shape->is_finite()) {
        finite_props_.push_back(prop.id);
    } else {
        infinite_props_.push_back(prop.id);
    }

    // Shape has no surface
    if (1 == shape->num_parts() && 1.f == materials[0].ptr->ior()) {
        if (shape->is_finite()) {
            volumes_.push_back(prop.id);
        } else {
            infinite_volumes_.push_back(prop.id);
        }
    }

    return prop.id;
}

uint32_t Scene::create_prop(Shape* shape, Materials const& materials,
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

void Scene::prop_attach(uint32_t parent_id, uint32_t child_id, uint32_t sibling_hint) noexcept {
    prop_detach_self(child_id);

    prop::Prop_topology& st = prop_topology_[parent_id];
    prop::Prop_topology& nt = prop_topology_[child_id];

    nt.parent = parent_id;

    if (0 == nt.num_local_frames) {
        // This is the case if n has no animation attached to it directly
        prop_allocate_frames(child_id, prop(parent_id)->num_world_frames(), 1);
    }

    if (prop::Null == st.child) {
        st.child = child_id;
    } else {
        if (prop::Null != sibling_hint && prop_topology_[sibling_hint].parent == parent_id) {
            prop_add_sibling(sibling_hint, child_id);
        } else {
            prop_add_sibling(st.child, child_id);
        }
    }
}

void Scene::prop_set_transformation(uint32_t entity, math::Transformation const& t) noexcept {
    props_[entity].set_transformation(t);
}

Scene::Transformation const& Scene::prop_world_transformation(uint32_t entity) const noexcept {
    return prop_world_transformations_[entity];
}

void Scene::prop_set_world_transformation(uint32_t entity, math::Transformation const& t) noexcept {
    prop_world_transformations_[entity].set(t);
}

void Scene::prop_allocate_frames(uint32_t entity, uint32_t num_world_frames,
                                 uint32_t num_local_frames) noexcept {
    props_[entity].allocate_frames(num_world_frames, num_local_frames);
    prop_topology_[entity].num_local_frames = num_local_frames;
}

void Scene::prop_set_frames(uint32_t entity, animation::Keyframe const* frames,
                            uint32_t num_frames) noexcept {
    props_[entity].set_frames(entity, frames, num_frames, *this);
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
    auto& prop = props_[entity];

    auto shape = prop.shape();

    shape->prepare_sampling(part);

    Transformation temp;
    auto const&    transformation = prop.transformation_at(entity, time, temp, *this);

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
    auto& prop = props_[entity];

    auto shape = prop.shape();

    shape->prepare_sampling(part);

    Transformation temp;
    auto const&    transformation = prop.transformation_at(entity, time, temp, *this);

    float const volume = shape->volume(part, transformation.scale);

    auto& m = prop_materials_[entity];

    m.parts[part].volume = volume;

    m.parts[part].light_id = light_id;

    material_resources_[m.materials[part]]->prepare_sampling(
        *shape, part, time, transformation, volume, material_importance_sampling, pool);
}

material::Material const* Scene::prop_material(uint32_t entity, uint32_t part) const noexcept {
    return material_resources_[prop_materials_[entity].materials[part]];
}

prop::Prop_topology const& Scene::prop_topology(uint32_t entity) const noexcept {
    return prop_topology_[entity];
}

uint32_t Scene::prop_light_id(uint32_t entity, uint32_t part) const noexcept {
    return prop_materials_[entity].parts[part].light_id;
}

float Scene::prop_area(uint32_t entity, uint32_t part) const noexcept {
    return prop_materials_[entity].parts[part].area;
}

float Scene::prop_volume(uint32_t entity, uint32_t part) const noexcept {
    return prop_materials_[entity].parts[part].volume;
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
    prop_topology_.emplace_back();

    uint32_t const prop_id = uint32_t(props_.size()) - 1;

    prop::Prop* prop = &props_[prop_id];

    return {prop, prop_id};
}

void Scene::prop_add_sibling(uint32_t self, uint32_t node) noexcept {
    for (; prop::Null != prop_topology_[self].next;) {
        self = prop_topology_[self].next;
    }

    prop_topology_[self].next = node;
}

void Scene::prop_detach_self(uint32_t self) noexcept {
    uint32_t const parent = prop_topology(self).parent;

    if (prop::Null != parent) {
        prop_detach(parent, self);
    }
}

void Scene::prop_detach(uint32_t self, uint32_t node) noexcept {
    // we can assume this to be true because of detach()
    // assert(node->parent_ == this);

    prop::Prop_topology& st = prop_topology_[self];
    prop::Prop_topology& nt = prop_topology_[node];

    nt.parent = prop::Null;

    if (st.child == node) {
        st.child = nt.next;
        nt.next  = prop::Null;
    } else {
        prop_remove_sibling(st.child, node);
    }
}

void Scene::prop_remove_sibling(uint32_t self, uint32_t node) noexcept {
    prop::Prop_topology& st = prop_topology_[self];
    prop::Prop_topology& nt = prop_topology_[node];

    if (st.next == node) {
        st.next = nt.next;
        nt.next = prop::Null;
    } else {
        prop_remove_sibling(st.next, node);
    }
}

bool Scene::prop_has_caustic_material(uint32_t entity) const noexcept {
    Prop const& p = props_[entity];
    for (uint32_t i = 0, len = p.shape()->num_parts(); i < len; ++i) {
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
