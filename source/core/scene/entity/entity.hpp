#ifndef SU_CORE_SCENE_ENTITY_ENTITY_HPP
#define SU_CORE_SCENE_ENTITY_ENTITY_HPP

#include "base/flags/flags.hpp"
#include "base/json/json_types.hpp"
#include "base/math/transformation.hpp"
#include "composed_transformation.hpp"
#include "keyframe.hpp"

namespace scene {

class Scene;

namespace entity {

class Entity {
  public:
    using Transformation = Composed_transformation;

    Entity() noexcept;

    Entity(Entity&& other) noexcept;

    virtual ~Entity() noexcept;

    virtual void set_parameters(json::Value const& parameters) noexcept = 0;

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

    void set_frames(Keyframe const* frames, uint32_t num_frames) noexcept;

    void calculate_world_transformation(Scene& scene) noexcept;

    bool visible_in_camera() const noexcept;
    bool visible_in_reflection() const noexcept;
    bool visible_in_shadow() const noexcept;

    void set_visible_in_shadow(bool value) noexcept;

    void set_visibility(bool in_camera, bool in_reflection, bool in_shadow) noexcept;

    void attach(uint32_t self, uint32_t node, Scene& scene) noexcept;

    void detach_self(uint32_t self, Scene& scene) noexcept;

  protected:
    void propagate_transformation(Scene& scene) noexcept;

    void inherit_transformation(Keyframe const* frames, uint32_t num_frames, Scene& scene) noexcept;

    void add_sibling(uint32_t node, Scene& scene) noexcept;

    void detach(uint32_t node, Scene& scene) noexcept;

    void remove_sibling(uint32_t node, Scene& scene) noexcept;

    virtual void on_set_transformation() noexcept = 0;

    enum class Property {
        Visible_in_camera     = 1 << 0,
        Visible_in_reflection = 1 << 1,
        Visible_in_shadow     = 1 << 2,
        Masked_material       = 1 << 3,
        Tinted_shadow         = 1 << 4
    };

    flags::Flags<Property> properties_;

    static uint32_t constexpr Null = 0xFFFFFFFF;

    uint32_t parent_ = Null;
    uint32_t next_   = Null;
    uint32_t child_  = Null;

    Transformation world_transformation_;

    uint32_t num_world_frames_ = 0;

    uint32_t num_local_frames_ = 0;

    Keyframe* frames_ = nullptr;
};

struct Entity_ref {
    Entity*  ref;
    uint32_t id;

    static Entity_ref constexpr Null() noexcept {
        return {nullptr, 0xFFFFFFFF};
    }
};

}  // namespace entity
}  // namespace scene

#endif
