#ifndef SU_CORE_SCENE_ENTITY_ENTITY_HPP
#define SU_CORE_SCENE_ENTITY_ENTITY_HPP

#include "base/flags/flags.hpp"
#include "base/json/json_types.hpp"
#include "base/math/transformation.hpp"
#include "composed_transformation.hpp"
#include "keyframe.hpp"

namespace scene::entity {

class Entity {
  public:
    using Transformation = Composed_transformation;

    Entity() noexcept = default;

    virtual ~Entity() noexcept;

    virtual void set_parameters(json::Value const& parameters) noexcept = 0;

    math::Transformation const& local_frame_a() const noexcept;

    // Only the returned reference is guaranteed to contain the actual transformation data.
    // This might or might not be the same reference which is passed as a parameter,
    // depending on whether the entity is animated or not.
    // This can sometimes avoid a relatively costly copy,
    // while keeping the animated state out of the interface.
    Transformation const& transformation_at(float tick_delta, Transformation& transformation) const
        noexcept;

    void set_transformation(math::Transformation const& t) noexcept;

    void tick(Keyframe const& frame) noexcept;

    void calculate_world_transformation() noexcept;

    bool visible_in_camera() const noexcept;
    bool visible_in_reflection() const noexcept;
    bool visible_in_shadow() const noexcept;

    void set_visible_in_shadow(bool value) noexcept;

    void set_visibility(bool in_camera, bool in_reflection, bool in_shadow,
                        bool propagate = false) noexcept;

    void set_propagate_visibility(bool enable) noexcept;

    void attach(Entity* node) noexcept;
    void detach() noexcept;

    Entity const* parent() const noexcept;

  protected:
    void propagate_transformation() const noexcept;

    void inherit_transformation(math::Transformation const& a, math::Transformation const& b,
                                bool animated) noexcept;

    void add_sibling(Entity* node) noexcept;
    void detach(Entity* node) noexcept;
    void remove_sibling(Entity* node) noexcept;

    virtual void on_set_transformation() noexcept = 0;

    enum class Property {
        Animated              = 1 << 0,
        Visible_in_camera     = 1 << 1,
        Visible_in_reflection = 1 << 2,
        Visible_in_shadow     = 1 << 3,
        Propagate_visibility  = 1 << 4,
        Masked_material       = 1 << 5,
        Tinted_shadow         = 1 << 6
    };

    flags::Flags<Property> properties_;

    Transformation world_transformation_;

    math::Transformation world_frame_a_;
    math::Transformation world_frame_b_;

    Keyframe local_frame_a_;
    Keyframe local_frame_b_;

    math::Transformation* world_frames_ = nullptr;

    Keyframe* local_frames_ = nullptr;

    Entity* parent_ = nullptr;
    Entity* next_   = nullptr;
    Entity* child_  = nullptr;
};

}  // namespace scene::entity

#endif
