#ifndef SU_CORE_SCENE_PROP_INTERFACE_STACK_HPP
#define SU_CORE_SCENE_PROP_INTERFACE_STACK_HPP

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
        material::Material const* material() const;

        bool matches(Intersection const& intersection) const;

        Prop const* prop;
        float2      uv;
        uint32_t    part;
    };

    Interface_stack();
    ~Interface_stack();

    void operator=(Interface_stack const& other);

    void swap(Interface_stack& other);

    bool empty() const;

    void clear();

    const Interface* top() const;

    bool top_is_vacuum() const;

    void push(Intersection const& intersection);
    bool remove(Intersection const& intersection);
    void pop();

  private:
    static int32_t constexpr Num_entries = 16;

    int32_t    index_;
    Interface* stack_;
};

}  // namespace prop
}  // namespace scene

#endif
