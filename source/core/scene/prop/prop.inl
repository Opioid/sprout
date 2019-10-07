#ifndef SU_CORE_SCENE_PROP_PROP_INL
#define SU_CORE_SCENE_PROP_PROP_INL

#include "prop.hpp"

namespace scene::prop {

inline uint32_t Prop::shape() const noexcept {
    return shape_;
}

inline bool Prop::has_local_animation() const noexcept {
    return properties_.is(Property::Local_animation);
}

inline bool Prop::has_no_parent() const noexcept {
    return properties_.no(Property::Has_parent);
}

inline void Prop::set_has_parent() noexcept {
    properties_.set(Property::Has_parent);
}

inline bool Prop::visible_in_camera() const noexcept {
    return properties_.is(Property::Visible_in_camera);
}

inline bool Prop::visible_in_reflection() const noexcept {
    return properties_.is(Property::Visible_in_reflection);
}

inline bool Prop::visible_in_shadow() const noexcept {
    return properties_.is(Property::Visible_in_shadow);
}

}  // namespace scene::prop

#endif
