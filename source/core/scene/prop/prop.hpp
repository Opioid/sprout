#ifndef SU_CORE_SCENE_PROP_PROP_HPP
#define SU_CORE_SCENE_PROP_PROP_HPP

#include "base/flags/flags.hpp"
#include "base/json/json_types.hpp"
#include "base/math/aabb.hpp"
#include "base/math/transformation.hpp"
#include "scene/animation/animation.hpp"
#include "scene/entity/composed_transformation.hpp"
#include "scene/entity/keyframe.hpp"
#include "scene/material/material.hpp"

namespace thread {
class Pool;
}

namespace scene {

class Scene;
class Worker;
struct Ray;

namespace shape {

struct Intersection;
struct Normals;
class Shape;
class Node_stack;
}  // namespace shape

namespace prop {

class alignas(64) Prop {
  public:
    using Transformation = entity::Composed_transformation;
    using Node_stack     = shape::Node_stack;
    using Filter         = material::Sampler_settings::Filter;
    using Material       = material::Material;
    using Shape          = shape::Shape;
    using Keyframe       = entity::Keyframe;

    Prop();

    Prop(Prop&& other) noexcept;

    ~Prop() noexcept;

    void allocate_frames(uint32_t num_world_frames, uint32_t num_local_frames) noexcept;

    math::Transformation const& local_frame_0() const noexcept;

    // Only the returned reference is guaranteed to contain the actual transformation data.
    // This might or might not be the same reference which is passed as a parameter,
    // depending on whether the entity is animated or not.
    // This can sometimes avoid a relatively costly copy,
    // while keeping the animated state out of the interface.
    Transformation const& transformation_at(uint64_t time, Transformation& transformation) const
        noexcept;

    void set_transformation(math::Transformation const& t) noexcept;

    void set_frames(animation::Keyframe const* frames, uint32_t num_frames) noexcept;

    void calculate_world_transformation(Scene& scene) noexcept;

    bool visible_in_camera() const noexcept;
    bool visible_in_reflection() const noexcept;
    bool visible_in_shadow() const noexcept;

    void set_visible_in_shadow(bool value) noexcept;

    void set_visibility(bool in_camera, bool in_reflection, bool in_shadow) noexcept;

    void attach(uint32_t self, uint32_t node, Scene& scene) noexcept;

    void detach_self(uint32_t self, Scene& scene) noexcept;

    void morph(thread::Pool& pool) noexcept;

    bool intersect(Ray& ray, Node_stack& node_stack, shape::Intersection& intersection) const
        noexcept;

    bool intersect_fast(Ray& ray, Node_stack& node_stack, shape::Intersection& intersection) const
        noexcept;

    bool intersect(Ray& ray, Node_stack& node_stack, shape::Normals& normals) const noexcept;

    bool intersect_p(Ray const& ray, shape::Node_stack& node_stack) const noexcept;

    shape::Shape const* shape() const noexcept;
    shape::Shape*       shape() noexcept;

    AABB const& aabb() const noexcept;

    void set_shape_and_materials(Shape* shape, Material* const* materials) noexcept;

    void set_parameters(json::Value const& parameters) noexcept;

    void prepare_sampling(uint32_t part, uint32_t light_id, uint64_t time,
                          bool material_importance_sampling, thread::Pool& pool) noexcept;

    void prepare_sampling_volume(uint32_t part, uint32_t light_id, uint64_t time,
                                 bool material_importance_sampling, thread::Pool& pool) noexcept;

    float opacity(Ray const& ray, Filter filter, Worker const& worker) const noexcept;

    bool thin_absorption(Ray const& ray, Filter filter, Worker const& worker, float3& ca) const
        noexcept;

    float area(uint32_t part) const noexcept;

    float volume(uint32_t part) const noexcept;

    uint32_t light_id(uint32_t part) const noexcept;

    material::Material const* material(uint32_t part) const noexcept;

    bool has_masked_material() const noexcept;
    bool has_caustic_material() const noexcept;
    bool has_tinted_shadow() const noexcept;
    bool has_no_surface() const noexcept;

    size_t num_bytes() const noexcept;

  private:
    void propagate_transformation(Scene& scene) noexcept;

    void inherit_transformation(Keyframe const* frames, uint32_t num_frames, Scene& scene) noexcept;

    void add_sibling(uint32_t node, Scene& scene) noexcept;

    void detach(uint32_t node, Scene& scene) noexcept;

    void remove_sibling(uint32_t node, Scene& scene) noexcept;

    void set_shape(Shape* shape) noexcept;

    bool visible(uint32_t ray_depth) const noexcept;

    void on_set_transformation() noexcept;

    enum class Property {
        Visible_in_camera     = 1 << 0,
        Visible_in_reflection = 1 << 1,
        Visible_in_shadow     = 1 << 2,
        Masked_material       = 1 << 3,
        Tinted_shadow         = 1 << 4,
        Test_AABB             = 1 << 5,
    };

    flags::Flags<Property> properties_;

    static uint32_t constexpr Null = 0xFFFFFFFF;

    uint32_t parent_ = Null;
    uint32_t next_   = Null;
    uint32_t child_  = Null;

    // Pre-transformed AABB in world space.
    // For moving objects it must cover the entire area occupied by the object during the tick.
    AABB aabb_;

    Transformation world_transformation_;

    uint32_t num_world_frames_ = 0;

    uint32_t num_local_frames_ = 0;

    entity::Keyframe* frames_ = nullptr;

    Shape* shape_ = nullptr;

    Material** materials_ = nullptr;

    struct Part {
        union {
            float area;
            float volume;
        };

        uint32_t light_id;
    };

    Part* parts_ = nullptr;

    entity::Morphing morphing_;
};

struct Prop_ref {
    Prop*    ref;
    uint32_t id;

    static Prop_ref constexpr Null() noexcept {
        return {nullptr, 0xFFFFFFFF};
    }
};

}  // namespace prop

using Prop     = prop::Prop;
using Prop_ref = prop::Prop_ref;

}  // namespace scene

#endif
