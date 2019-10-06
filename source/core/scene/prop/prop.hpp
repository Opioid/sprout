#ifndef SU_CORE_SCENE_PROP_PROP_HPP
#define SU_CORE_SCENE_PROP_PROP_HPP

#include "base/flags/flags.hpp"
#include "base/json/json_types.hpp"
#include "base/math/aabb.hpp"
#include "base/math/transformation.hpp"
#include "scene/entity/composed_transformation.hpp"
#include "scene/entity/keyframe.hpp"
#include "scene/material/material.hpp"

namespace resource {

template <typename T>
struct Resource_ptr;

}

namespace thread {
class Pool;
}

namespace scene {

class Scene;
class Worker;
struct Ray;

namespace animation {
struct Keyframe;
}

namespace shape {

struct Intersection;
struct Normals;
class Shape;
class Node_stack;
}  // namespace shape

namespace prop {

static uint32_t constexpr Null = 0xFFFFFFFF;

class Prop {
  public:
    using Transformation = entity::Composed_transformation;
    using Node_stack     = shape::Node_stack;
    using Filter         = material::Sampler_settings::Filter;
    using Material       = material::Material;
    using Material_ptr   = resource::Resource_ptr<Material>;
    using Shape          = shape::Shape;
    using Shape_ptr      = resource::Resource_ptr<Shape>;
    using Keyframe       = entity::Keyframe;

    Prop() noexcept;

    ~Prop() noexcept;

    void allocate_frames(uint32_t self, uint32_t num_world_frames, Scene const& scene) noexcept;

    uint32_t shape() const noexcept;

    void configure(Shape_ptr shape, Material_ptr const* materials) noexcept;

    bool has_no_parent() const noexcept;

    bool visible_in_camera() const noexcept;

    bool visible_in_reflection() const noexcept;

    bool visible_in_shadow() const noexcept;

    void set_visible_in_shadow(bool value) noexcept;

    void set_visibility(bool in_camera, bool in_reflection, bool in_shadow) noexcept;

    void set_has_parent() noexcept;

    void morph(uint32_t self, thread::Pool& pool, Scene const& scene) noexcept;

    bool intersect(uint32_t self, Ray& ray, Worker const& worker,
                   shape::Intersection& intersection) const noexcept;

    bool intersect_fast(uint32_t self, Ray& ray, Worker const& worker,
                        shape::Intersection& intersection) const noexcept;

    bool intersect(uint32_t self, Ray& ray, Worker const& worker, shape::Normals& normals) const
        noexcept;

    bool intersect_p(uint32_t self, Ray const& ray, Worker const& worker) const noexcept;

    float opacity(uint32_t self, Ray const& ray, Filter filter, Worker const& worker) const
        noexcept;

    bool thin_absorption(uint32_t self, Ray const& ray, Filter filter, Worker const& worker,
                         float3& ca) const noexcept;

    bool has_masked_material() const noexcept;

    bool has_tinted_shadow() const noexcept;

    size_t num_bytes() const noexcept;

  private:
    void set_shape(uint32_t shape) noexcept;

    bool visible(uint32_t ray_depth) const noexcept;

    enum class Property {
        Visible_in_camera     = 1 << 0,
        Visible_in_reflection = 1 << 1,
        Visible_in_shadow     = 1 << 2,
        Masked_material       = 1 << 3,
        Tinted_shadow         = 1 << 4,
        Test_AABB             = 1 << 5,
        Has_parent            = 1 << 6,
        Static                = 1 << 7
    };

    flags::Flags<Property> properties_;

    uint32_t shape_ = 0xFFFFFFFF;
};

struct Prop_ptr {
    Prop*    ptr;
    uint32_t id;

    static Prop_ptr constexpr Null() noexcept {
        return {nullptr, 0xFFFFFFFF};
    }
};

struct alignas(16) Prop_material {
    Prop_material() noexcept;

    Prop_material(Prop_material&& other) noexcept;

    ~Prop_material() noexcept;

    uint32_t* materials;

    struct Part {
        union {
            float area;
            float volume;
        };

        uint32_t light_id;
    };

    Part* parts;
};

struct alignas(16) Prop_frames {
    Prop_frames() noexcept;

    Prop_frames(Prop_frames&& other) noexcept;

    ~Prop_frames() noexcept;

    uint32_t num_world_frames;
    uint32_t num_local_frames;

    entity::Keyframe* frames;
};

struct alignas(8) Prop_topology {
    uint32_t next  = Null;
    uint32_t child = Null;
};

}  // namespace prop

}  // namespace scene

#endif
