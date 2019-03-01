#include "scene.hpp"
#include "animation/animation.hpp"
#include "animation/animation_stage.hpp"
#include "base/math/aabb.inl"
#include "base/math/distribution/distribution_1d.inl"
#include "base/math/matrix3x3.inl"
#include "base/math/quaternion.inl"
#include "base/math/vector3.inl"
#include "base/spectrum/rgb.hpp"
#include "bvh/scene_bvh_builder.inl"
#include "entity/dummy.hpp"
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

Scene::Scene() noexcept {
    dummies_.reserve(16);
    finite_props_.reserve(16);
    infinite_props_.reserve(2);
    volumes_.reserve(16);
    infinite_volumes_.reserve(1);
    lights_.reserve(16);
    extensions_.reserve(16);
    entities_.reserve(16);
    light_powers_.reserve(16);
    materials_.reserve(16);
    animations_.reserve(16);
    animation_stages_.reserve(16);
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

    for (auto p : finite_props_) {
        delete p;
    }

    for (auto p : infinite_props_) {
        delete p;
    }

    for (auto const d : dummies_) {
        delete d;
    }
}

void Scene::finish(uint64_t frame_step, uint64_t frame_duration) noexcept {
    if (lights_.empty()) {
        lights_.push_back(&null_light_);
    }

    uint32_t const num_frames = count_frames(frame_step, frame_duration) + 1;

    for (auto a : animations_) {
        a->allocate_interpolated_frames(num_frames);
    }

    for (auto& s : animation_stages_) {
        s.allocate_enitity_frames();
    }

    for (auto e : entities_) {
        e->propagate_frame_allocation();
    }
}

AABB const& Scene::aabb() const noexcept {
    return prop_bvh_.aabb();
}

AABB Scene::caustic_aabb() const noexcept {
    AABB aabb = AABB::empty();

    for (auto const p : finite_props_) {
        if (p->has_caustic_material()) {
            aabb.merge_assign(p->aabb());
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

bool Scene::intersect(Ray& ray, Node_stack& node_stack) const noexcept {
    return prop_bvh_.intersect(ray, node_stack);
}

bool Scene::intersect_volume(Ray& ray, Node_stack& node_stack,
                             prop::Intersection& intersection) const noexcept {
    return volume_bvh_.intersect_fast(ray, node_stack, intersection);
}

bool Scene::intersect_p(Ray const& ray, Node_stack& node_stack) const noexcept {
    return prop_bvh_.intersect_p(ray, node_stack);
}

bool Scene::opacity(Ray const& ray, Filter filter, Worker const& worker, float& o) const noexcept {
    if (has_masked_material_) {
        return prop_bvh_.opacity(ray, filter, worker, o);
    }

    bool const visible = !prop_bvh_.intersect_p(ray, worker.node_stack());

    o = visible ? 0.f : 1.f;

    return visible;
}

bool Scene::thin_absorption(Ray const& ray, Filter filter, Worker const& worker, float3& ta) const
    noexcept {
    if (has_tinted_shadow_) {
        return prop_bvh_.thin_absorption(ray, filter, worker, ta);
    }

    float      o;
    bool const visible = opacity(ray, filter, worker, o);

    ta = float3(o);

    return visible;
}

entity::Entity* Scene::entity(size_t index) const noexcept {
    if (index >= entities_.size()) {
        return nullptr;
    }

    return entities_[index];
}

entity::Entity* Scene::entity(std::string_view name) const noexcept {
    auto e = named_entities_.find(name);
    if (named_entities_.end() == e) {
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
    return {*lights_[id], pdf};
}

Scene::Light Scene::random_light(float random) const noexcept {
    SOFT_ASSERT(!lights_.empty());

    auto const l = light_distribution_.sample_discrete(random);

    SOFT_ASSERT(l.offset < static_cast<uint32_t>(lights_.size()));

    return {*lights_[l.offset], l.pdf};
}

void Scene::simulate(uint64_t start, uint64_t end, thread::Pool& thread_pool) noexcept {
    uint64_t const frames_start = start - (start % tick_duration_);
    uint64_t const end_rem      = end % tick_duration_;
    uint64_t const frames_end   = end + (end_rem ? tick_duration_ - end_rem : 0);

    for (auto a : animations_) {
        a->resample(frames_start, frames_end, tick_duration_);
    }

    for (auto& s : animation_stages_) {
        s.update();
    }

    for (auto p : finite_props_) {
        p->morph(thread_pool);
    }

    for (auto m : materials_) {
        m->simulate(start, end, tick_duration_, thread_pool);
    }

    compile(start, thread_pool);
}

void Scene::compile(uint64_t time, thread::Pool& pool) noexcept {
    has_masked_material_ = false;
    has_tinted_shadow_   = false;

    // handle changed transformations
    for (auto const d : dummies_) {
        d->calculate_world_transformation();
    }

    for (auto e : extensions_) {
        e->calculate_world_transformation();
    }

    for (auto p : finite_props_) {
        p->calculate_world_transformation();
        has_masked_material_ = has_masked_material_ || p->has_masked_material();
        has_tinted_shadow_   = has_tinted_shadow_ || p->has_tinted_shadow();
    }

    for (auto p : infinite_props_) {
        p->calculate_world_transformation();
        has_masked_material_ = has_masked_material_ || p->has_masked_material();
        has_tinted_shadow_   = has_tinted_shadow_ || p->has_tinted_shadow();
    }

    for (auto v : volumes_) {
        v->set_visible_in_shadow(false);
    }

    // rebuild prop BVH
    bvh_builder_.build(prop_bvh_.tree(), finite_props_);
    prop_bvh_.set_infinite_props(infinite_props_);

    // rebuild volume BVH
    bvh_builder_.build(volume_bvh_.tree(), volumes_);
    volume_bvh_.set_infinite_props(infinite_volumes_);

    // re-sort lights PDF
    light_powers_.clear();

    for (uint32_t i = 0, len = static_cast<uint32_t>(lights_.size()); i < len; ++i) {
        auto l = lights_[i];
        l->prepare_sampling(i, time, pool);
        light_powers_.push_back(std::sqrt(spectrum::luminance(l->power(prop_bvh_.aabb()))));
    }

    light_distribution_.init(light_powers_.data(), light_powers_.size());

    has_volumes_ = !volumes_.empty() || !infinite_volumes_.empty();
}

entity::Dummy* Scene::create_dummy() noexcept {
    entity::Dummy* dummy = new entity::Dummy;
    dummies_.push_back(dummy);

    entities_.push_back(dummy);

    return dummy;
}

entity::Dummy* Scene::create_dummy(std::string const& name) noexcept {
    entity::Dummy* dummy = create_dummy();

    add_named_entity(dummy, name);

    return dummy;
}

Prop* Scene::create_prop(Shape* shape, Materials const& materials) noexcept {
    prop::Prop* prop = new prop::Prop;

    prop->set_shape_and_materials(shape, materials.data());

    if (shape->is_finite()) {
        finite_props_.push_back(prop);
    } else {
        infinite_props_.push_back(prop);
    }

    if (prop->has_no_surface()) {
        if (shape->is_finite()) {
            volumes_.push_back(prop);
        } else {
            infinite_volumes_.push_back(prop);
        }
    }

    entities_.push_back(prop);

    return prop;
}

prop::Prop* Scene::create_prop(Shape* shape, Materials const& materials,
                               std::string const& name) noexcept {
    prop::Prop* prop = create_prop(shape, materials);

    add_named_entity(prop, name);

    return prop;
}

light::Light* Scene::create_prop_light(Prop* prop, uint32_t part) noexcept {
    light::Prop_light* light = new light::Prop_light;

    lights_.push_back(light);

    light->init(prop, part);

    return light;
}

light::Light* Scene::create_prop_image_light(Prop* prop, uint32_t part) noexcept {
    light::Prop_image_light* light = new light::Prop_image_light;

    lights_.push_back(light);

    light->init(prop, part);

    return light;
}

light::Light* Scene::create_prop_volume_light(Prop* prop, uint32_t part) noexcept {
    light::Prop_volume_light* light = new light::Prop_volume_light;

    lights_.push_back(light);

    light->init(prop, part);

    return light;
}

light::Light* Scene::create_prop_volume_image_light(Prop* prop, uint32_t part) noexcept {
    light::Prop_volume_image_light* light = new light::Prop_volume_image_light;

    lights_.push_back(light);

    light->init(prop, part);

    return light;
}

void Scene::add_extension(Entity* extension) noexcept {
    extensions_.push_back(extension);

    entities_.push_back(extension);
}

void Scene::add_extension(Entity* extension, std::string const& name) noexcept {
    add_extension(extension);

    add_named_entity(extension, name);
}

void Scene::add_material(Material* material) noexcept {
    materials_.push_back(material);
}

animation::Animation* Scene::create_animation(uint32_t count) noexcept {
    animation::Animation* animation = new animation::Animation(count);

    animations_.push_back(animation);

    return animation;
}

void Scene::create_animation_stage(Entity* entity, animation::Animation* animation) noexcept {
    animation_stages_.push_back(animation::Stage(entity, animation));
}

size_t Scene::num_bytes() const noexcept {
    size_t num_bytes = 0;

    for (auto p : finite_props_) {
        num_bytes += p->num_bytes();
    }

    for (auto p : infinite_props_) {
        num_bytes += p->num_bytes();
    }

    return num_bytes + sizeof(*this);
}

void Scene::add_named_entity(Entity* entity, std::string const& name) noexcept {
    if (!entity || name.empty()) {
        return;
    }

    if (named_entities_.find(name) != named_entities_.end()) {
        return;
    }

    named_entities_.insert_or_assign(name, entity);
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
