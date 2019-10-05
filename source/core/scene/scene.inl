#ifndef SU_CORE_SCENE_SCENE_INL
#define SU_CORE_SCENE_SCENE_INL

#include "base/math/aabb.inl"
#include "prop/prop.hpp"
#include "scene.hpp"
#include "scene_ray.hpp"

#include "base/debug/assert.hpp"

namespace scene {

inline Scene::Transformation const& Scene::prop_world_transformation(uint32_t entity) const
    noexcept {
    return prop_world_transformations_[entity];
}

inline void Scene::prop_set_world_transformation(uint32_t                    entity,
                                                 math::Transformation const& t) noexcept {
    prop_world_transformations_[entity].set(t);
}

inline math::Transformation const& Scene::prop_local_frame_0(uint32_t entity) const noexcept {
    prop::Prop_frames const& f = prop_frames_[entity];

    return f.frames[f.num_world_frames].transformation;
}

inline bool Scene::prop_aabb_intersect_p(uint32_t entity, Ray const& ray) const noexcept {
    return prop_aabbs_[entity].intersect_p(ray);
}

inline shape::Shape* Scene::prop_shape(uint32_t entity) const noexcept {
    return shape_resources_[props_[entity].shape()];
}

inline material::Material const* Scene::prop_material(uint32_t entity, uint32_t part) const
    noexcept {
    return material_resources_[prop_materials_[entity].materials[part]];
}

inline prop::Prop_topology const& Scene::prop_topology(uint32_t entity) const noexcept {
    return prop_topology_[entity];
}

inline uint32_t Scene::prop_light_id(uint32_t entity, uint32_t part) const noexcept {
    return prop_materials_[entity].parts[part].light_id;
}

inline float Scene::prop_area(uint32_t entity, uint32_t part) const noexcept {
    return prop_materials_[entity].parts[part].area;
}

inline float Scene::prop_volume(uint32_t entity, uint32_t part) const noexcept {
    return prop_materials_[entity].parts[part].volume;
}

inline prop::Prop const* Scene::prop(uint32_t index) const noexcept {
    SOFT_ASSERT(index < props_.size());

    return &props_[index];
}

inline prop::Prop* Scene::prop(uint32_t index) noexcept {
    SOFT_ASSERT(index < props_.size());

    return &props_[index];
}

inline prop::Prop* Scene::prop(std::string_view name) noexcept {
    auto e = named_props_.find(name);
    if (named_props_.end() == e) {
        return nullptr;
    }

    return &props_[e->second];
}

}  // namespace scene

#endif
