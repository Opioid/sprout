#ifndef SU_CORE_SCENE_PROP_INTERFACE_STACK_INL
#define SU_CORE_SCENE_PROP_INTERFACE_STACK_INL

#include "base/memory/align.hpp"
#include "interface_stack.hpp"
#include "prop.hpp"
#include "prop_intersection.hpp"

#include "base/debug/assert.hpp"

namespace scene::prop {

inline material::Material const* Interface_stack::Interface::material() const noexcept {
    return prop->material(part);
}

inline bool Interface_stack::Interface::matches(Intersection const& intersection) const noexcept {
    return prop == intersection.prop && part == intersection.geo.part;
}

inline Interface_stack::Interface_stack() noexcept
    : stack_(memory::allocate_aligned<Interface>(Num_entries)) {}

inline Interface_stack::~Interface_stack() noexcept {
    memory::free_aligned(stack_);
}

inline void Interface_stack::operator=(Interface_stack const& other) noexcept {
    index_ = other.index_;

    for (int32_t i = 0, len = index_; i < len; ++i) {
        stack_[i] = other.stack_[i];
    }
}

inline void Interface_stack::swap(Interface_stack& other) noexcept {
    Interface* temp = stack_;
    stack_          = other.stack_;
    other.stack_    = temp;

    index_ = other.index_;
}

inline bool Interface_stack::empty() const noexcept {
    return 0 == index_;
}

inline void Interface_stack::clear() noexcept {
    index_ = 0;
}

inline Interface_stack::Interface const* Interface_stack::top() const noexcept {
    if (index_ > 0) {
        return &stack_[index_ - 1];
    }

    return nullptr;
}

inline float Interface_stack::top_ior() const noexcept {
    if (index_ > 0) {
        return stack_[index_ - 1].material()->ior();
    }

    return 1.f;
}

inline bool Interface_stack::top_is_vacuum() const noexcept {
    if (index_ > 0) {
        return 1.f == stack_[index_ - 1].material()->ior();
    }

    return true;
}

inline bool Interface_stack::top_is_vacuum_or_pure_specular() const noexcept {
    if (index_ > 0) {
        auto const material = stack_[index_ - 1].material();
        return 1.f == material->ior() || material->is_pure_specular();
    }

    return true;
}

inline void Interface_stack::push(Intersection const& intersection) noexcept {
    if (index_ < Num_entries - 1) {
        stack_[index_] = {intersection.prop, intersection.geo.uv, intersection.geo.part};
        ++index_;
    }
}

inline bool Interface_stack::remove(Intersection const& intersection) noexcept {
    int32_t const back = index_ - 1;
    for (int32_t i = back; i >= 0; --i) {
        if (stack_[i].matches(intersection)) {
            for (int32_t j = i; j < back; ++j) {
                stack_[j] = stack_[j + 1];
            }

            --index_;
            return true;
        }
    }

    return false;
}

inline float Interface_stack::peek_ior(Intersection const& intersection) const noexcept {
    if (index_ <= 1) {
        return 1.f;
    }

    int32_t const back = index_ - 1;
    if (stack_[back].matches(intersection)) {
        return stack_[back - 1].material()->ior();
    } else {
        return stack_[back].material()->ior();
    }
}

inline void Interface_stack::pop() noexcept {
    if (index_ > 0) {
        --index_;
    }
}

inline size_t Interface_stack::num_bytes() const noexcept {
    return sizeof(*this) + Num_entries * sizeof(Interface);
}

}  // namespace scene::prop

#endif
