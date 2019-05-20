#include "scene.hpp"
#include "animation/animation.hpp"
#include "animation/animation_stage.hpp"
#include "base/math/aabb.inl"
#include "base/math/distribution/distribution_1d.inl"
#include "base/math/matrix3x3.inl"
#include "base/math/quaternion.inl"
#include "base/math/vector3.inl"
#include "base/memory/array.inl"
#include "base/spectrum/rgb.hpp"
#include "bvh/scene_bvh_builder.inl"
#include "extension.hpp"
#include "image/texture/texture.hpp"
#include "light/prop_image_light.hpp"
#include "light/prop_light.hpp"
#include "light/prop_volume_image_light.hpp"
#include "light/prop_volume_light.hpp"
#include "prop/prop.hpp"
#include "prop/prop_intersection.hpp"
#include "scene_constants.hpp"
#include "scene_ray.hpp"
#include "scene_worker.hpp"
#include "shape/shape.hpp"

#include "base/debug/assert.hpp"

namespace scene {

static size_t constexpr Num_reserved_props = 32;

Scene::Scene() noexcept {
    props_.reserve(Num_reserved_props);
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
    for (auto a : animations_) {
        delete a;
    }

    for (auto e : extensions_) {
        delete e;
    }

    // Normally lights_ should never be empty; containing null_light instead
    // But it can happen for partially constructed scenes,
    // and we still don't want the destructor to crash.
    if (!lights_.empty() && lights_[0] != &null_light_) {
        for (auto l : lights_) {
            delete l;
        }
    }
}

void Scene::finish() noexcept {
    if (lights_.empty()) {
        lights_.push_back(&null_light_);
    }

    light_powers_.resize(lights_.size());
}

AABB const& Scene::aabb() const noexcept {
    return prop_bvh_.aabb();
}

AABB Scene::caustic_aabb() const noexcept {
    AABB aabb = AABB::empty();

    for (auto const i : finite_props_) {
        Prop const& p = props_[i];
        if (p.has_caustic_material()) {
            aabb.merge_assign(p.aabb());
        }
    }

    return aabb;
}

AABB Scene::caustic_aabb(float3x3 const& rotation) const noexcept {
    AABB aabb = AABB::empty();

    for (auto const i : finite_props_) {
        Prop const& p = props_[i];
        if (p.has_caustic_material()) {
            aabb.merge_assign(p.aabb().transform_transposed(rotation));
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

bool Scene::intersect(Ray& ray, Node_stack& node_stack, prop::Intersection& intersection) const
    noexcept {
    return prop_bvh_.intersect(ray, node_stack, intersection);
}

bool Scene::intersect(Ray& ray, Node_stack& node_stack, shape::Normals& normals) const noexcept {
    return prop_bvh_.intersect(ray, node_stack, normals);
}

bool Scene::intersect_volume(Ray& ray, Node_stack& node_stack,
                             prop::Intersection& intersection) const noexcept {
    return volume_bvh_.intersect_fast(ray, node_stack, intersection);
}

bool Scene::intersect_p(Ray const& ray, Node_stack& node_stack) const noexcept {
    return prop_bvh_.intersect_p(ray, node_stack);
}

bool Scene::visibility(Ray const& ray, Filter filter, Worker const& worker, float& v) const
    noexcept {
    if (has_masked_material_) {
        return prop_bvh_.visibility(ray, filter, worker, v);
    }

    if (!prop_bvh_.intersect_p(ray, worker.node_stack())) {
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

Prop const* Scene::prop(size_t index) const noexcept {
    SOFT_ASSERT(index < props_.size());

    return &props_[index];
}

Prop* Scene::prop(size_t index) noexcept {
    SOFT_ASSERT(index < props_.size());

    return &props_[index];
}

Prop* Scene::prop(std::string_view name) const noexcept {
    auto e = named_props_.find(name);
    if (named_props_.end() == e) {
        return nullptr;
    }

    return e->second;
}

std::vector<light::Light*> const& Scene::lights() const noexcept {
    return lights_;
}

Scene::Light Scene::light(uint32_t id, bool calculate_pdf) const noexcept {
    // If the assert doesn't hold it would pose a problem,
    // but I think it is more efficient to handle those cases outside or implicitely.
    SOFT_ASSERT(!lights_.empty() && light::Light::is_light(id));

    id = light::Light::strip_mask(id);

    float const pdf = calculate_pdf ? light_distribution_.pdf(id) : 1.f;
    return {*lights_[id], pdf, id};
}

Scene::Light Scene::random_light(float random) const noexcept {
    SOFT_ASSERT(!lights_.empty());

    auto const l = light_distribution_.sample_discrete(random);

    SOFT_ASSERT(l.offset < static_cast<uint32_t>(lights_.size()));

    return {*lights_[l.offset], l.pdf, l.offset};
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

    for (auto& p : props_) {
        p.morph(thread_pool);
    }

    for (auto m : materials_) {
        m->simulate(start, end, tick_duration_, thread_pool);
    }

    compile(start, thread_pool);
}

void Scene::compile(uint64_t time, thread::Pool& pool) noexcept {
    has_masked_material_ = false;
    has_tinted_shadow_   = false;

    for (auto e : extensions_) {
        e->update(*this);
    }

    for (auto& p : props_) {
        p.calculate_world_transformation(*this);
        has_masked_material_ = has_masked_material_ || p.has_masked_material();
        has_tinted_shadow_   = has_tinted_shadow_ || p.has_tinted_shadow();
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
    for (uint32_t i = 0, len = static_cast<uint32_t>(lights_.size()); i < len; ++i) {
        auto l = lights_[i];
        l->prepare_sampling(i, time, *this, pool);
        light_powers_[i] = std::sqrt(spectrum::luminance(l->power(prop_bvh_.aabb(), *this)));
    }

    light_distribution_.init(light_powers_.data(), static_cast<uint32_t>(light_powers_.size()));

    has_volumes_ = !volumes_.empty() || !infinite_volumes_.empty();
}

void Scene::calculate_num_interpolation_frames(uint64_t frame_step,
                                               uint64_t frame_duration) noexcept {
    num_interpolation_frames_ = count_frames(frame_step, frame_duration) + 1;
}

Scene::Prop_ref Scene::create_dummy() noexcept {
    props_.emplace_back();

    uint32_t const prop_id = static_cast<uint32_t>(props_.size()) - 1;

    prop::Prop* prop = &props_[prop_id];

    prop->set_shape_and_materials(&null_shape_, nullptr);

    return {prop, prop_id};
}

Scene::Prop_ref Scene::create_dummy(std::string const& name) noexcept {
    Prop_ref dummy = create_dummy();

    add_named_prop(dummy.ref, name);

    return dummy;
}

Scene::Prop_ref Scene::create_prop(Shape* shape, Materials const& materials) noexcept {
    props_.emplace_back();

    uint32_t const prop_id = static_cast<uint32_t>(props_.size()) - 1;

    prop::Prop* prop = &props_[prop_id];

    prop->set_shape_and_materials(shape, materials.data());

    if (shape->is_finite()) {
        finite_props_.push_back(prop_id);
    } else {
        infinite_props_.push_back(prop_id);
    }

    if (prop->has_no_surface()) {
        if (shape->is_finite()) {
            volumes_.push_back(prop_id);
        } else {
            infinite_volumes_.push_back(prop_id);
        }
    }

    return {prop, prop_id};
}

Scene::Prop_ref Scene::create_prop(Shape* shape, Materials const& materials,
                                   std::string const& name) noexcept {
    Prop_ref prop = create_prop(shape, materials);

    add_named_prop(prop.ref, name);

    return prop;
}

light::Light* Scene::create_prop_light(uint32_t prop, uint32_t part) noexcept {
    light::Prop_light* light = new light::Prop_light;

    lights_.push_back(light);

    light->init(prop, part);

    return light;
}

light::Light* Scene::create_prop_image_light(uint32_t prop, uint32_t part) noexcept {
    light::Prop_image_light* light = new light::Prop_image_light;

    lights_.push_back(light);

    light->init(prop, part);

    return light;
}

light::Light* Scene::create_prop_volume_light(uint32_t prop, uint32_t part) noexcept {
    light::Prop_volume_light* light = new light::Prop_volume_light;

    lights_.push_back(light);

    light->init(prop, part);

    return light;
}

light::Light* Scene::create_prop_volume_image_light(uint32_t prop, uint32_t part) noexcept {
    light::Prop_volume_image_light* light = new light::Prop_volume_image_light;

    lights_.push_back(light);

    light->init(prop, part);

    return light;
}

Scene::Prop_ref Scene::create_extension(Extension* extension) noexcept {
    extensions_.push_back(extension);

    Prop_ref dummy = create_dummy();

    extension->init(dummy.id);

    return dummy;
}

void Scene::prop_attach(uint32_t parent_id, uint32_t child_id) noexcept {
    props_[parent_id].attach(parent_id, child_id, *this);
}

void Scene::prop_set_transformation(uint32_t entity, math::Transformation const& t) noexcept {
    props_[entity].set_transformation(t);
}

void Scene::prop_allocate_frames(uint32_t entity, uint32_t num_world_frames,
                                 uint32_t num_local_frames) noexcept {
    props_[entity].allocate_frames(num_world_frames, num_local_frames);
}

void Scene::add_material(Material* material) noexcept {
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

void Scene::add_named_prop(Prop* prop, std::string const& name) noexcept {
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
    uint32_t const a = std::max(static_cast<uint32_t>(frame_duration / tick_duration_), 1u);
    uint32_t const b = matching(frame_step, tick_duration_) ? 0 : 1;
    uint32_t const c = matching(frame_duration, tick_duration_) ? 0 : 1;

    return a + b + c;
}

}  // namespace scene
