#ifndef SU_CORE_SCENE_PROP_INTERFACE_STACK_HPP
#define SU_CORE_SCENE_PROP_INTERFACE_STACK_HPP

#include <cstddef>
#include <cstdint>
#include "base/math/vector2.hpp"

namespace scene {

namespace material {
class Material;
}

namespace prop {

struct Intersection;
class Prop;

class Interface_stack {
  public:
    struct Interface {
        material::Material const* material() const noexcept;

        bool matches(Intersection const& intersection) const noexcept;

        Prop const* prop;
        float2      uv;
        uint32_t    part;
    };

    Interface_stack() noexcept;

    Interface_stack(Interface_stack const& other) noexcept;

    ~Interface_stack() noexcept;

    void operator=(Interface_stack const& other) noexcept;

    void swap(Interface_stack& other) noexcept;

    bool empty() const noexcept;

    void clear() noexcept;

    Interface const* top() const noexcept;

    float top_ior() const noexcept;

    bool top_is_vacuum() const noexcept;
    bool top_is_vacuum_or_not_scattering() const noexcept;

    void push(Intersection const& intersection) noexcept;

    bool remove(Intersection const& intersection) noexcept;

    float peek_ior(Intersection const& intersection) const noexcept;

    void pop() noexcept;

    size_t num_bytes() const noexcept;

  private:
    static int32_t constexpr Num_entries = 16;

    int32_t    index_;
    Interface* stack_;
};

}  // namespace prop
}  // namespace scene

#endif
