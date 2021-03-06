#ifndef SU_CORE_SCENE_PROP_INTERFACE_STACK_INL
#define SU_CORE_SCENE_PROP_INTERFACE_STACK_INL

#include "interface_stack.hpp"
#include "prop.hpp"
#include "prop_intersection.hpp"
#include "scene/material/material.hpp"
#include "scene/scene.hpp"
#include "scene/scene_worker.hpp"

#include "base/debug/assert.hpp"

namespace scene::prop {

inline material::Material const* Interface::material(Worker const& worker) const {
    return worker.scene().prop_material(prop, part);
}

inline bool Interface::matches(Intersection const& isec) const {
    return (prop == isec.prop) & (part == isec.geo.part);
}

inline Interface_stack::Interface_stack() : index_(0), stack_(new Interface[Num_entries]) {}

inline Interface_stack::Interface_stack(Interface_stack const& other)
    : stack_(new Interface[Num_entries]) {
    *this = other;
}

inline Interface_stack::~Interface_stack() {
    delete[] stack_;
}

inline void Interface_stack::operator=(Interface_stack const& other) {
    index_ = other.index_;

    for (int32_t i = 0, len = index_; i < len; ++i) {
        stack_[i] = other.stack_[i];
    }
}

inline void Interface_stack::swap(Interface_stack& other) {
    Interface* temp = stack_;
    stack_          = other.stack_;
    other.stack_    = temp;

    index_ = other.index_;
}

inline bool Interface_stack::empty() const {
    return 0 == index_;
}

inline void Interface_stack::clear() {
    index_ = 0;
}

inline Interface const* Interface_stack::top() const {
    if (index_ > 0) {
        return &stack_[index_ - 1];
    }

    return nullptr;
}

inline float Interface_stack::top_ior(Worker const& worker) const {
    if (index_ > 0) {
        return stack_[index_ - 1].material(worker)->ior();
    }

    return 1.f;
}

inline float Interface_stack::next_to_bottom_ior(Worker const& worker) const {
    if (index_ > 1) {
        return stack_[1].material(worker)->ior();
    }

    return 1.f;
}

inline bool Interface_stack::straight(Worker const& worker) const {
    if (index_ > 0) {
        return 1.f == stack_[index_ - 1].material(worker)->ior();
    }

    return true;
}

inline void Interface_stack::push(Intersection const& isec) {
    SOFT_ASSERT(index_ < Num_entries - 1);

    if (index_ < Num_entries - 1) {
        stack_[index_] = {isec.prop, isec.geo.part, isec.geo.uv};
        ++index_;
    }
}

inline bool Interface_stack::remove(Intersection const& isec) {
    int32_t const back = index_ - 1;
    for (int32_t i = back; i >= 0; --i) {
        if (stack_[i].matches(isec)) {
            for (int32_t j = i; j < back; ++j) {
                stack_[j] = stack_[j + 1];
            }

            --index_;
            return true;
        }
    }

    return false;
}

inline float Interface_stack::peek_ior(Intersection const& isec, Worker const& worker) const {
    if (index_ <= 1) {
        return 1.f;
    }

    int32_t const back = index_ - 1;
    if (stack_[back].matches(isec)) {
        return stack_[back - 1].material(worker)->ior();
    } else {
        return stack_[back].material(worker)->ior();
    }
}

inline void Interface_stack::pop() {
    if (index_ > 0) {
        --index_;
    }
}

}  // namespace scene::prop

#endif
