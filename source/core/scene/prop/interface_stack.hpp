#ifndef SU_CORE_SCENE_PROP_INTERFACE_STACK_HPP
#define SU_CORE_SCENE_PROP_INTERFACE_STACK_HPP

#include "base/math/vector2.hpp"

#include <cstdint>

namespace scene {

class Worker;

namespace material {
class Material;
}

namespace prop {

struct Intersection;
class Prop;

struct alignas(16) Interface {
    material::Material const* material(Worker const& worker) const;

    bool matches(Intersection const& intersection) const;

    uint32_t prop;
    uint32_t part;
    float2   uv;
};

class Interface_stack {
  public:
    Interface_stack();

    Interface_stack(Interface_stack const& other);

    ~Interface_stack();

    void operator=(Interface_stack const& other);

    void swap(Interface_stack& other);

    bool empty() const;

    bool empty_or_scattering(Worker const& worker) const;

    void clear();

    Interface const* top() const;

    float top_ior(Worker const& worker) const;

    float next_to_bottom_ior(Worker const& worker) const;

    bool straight(Worker const& worker) const;

    void push(Intersection const& intersection);

    bool remove(Intersection const& intersection);

    float peek_ior(Intersection const& intersection, Worker const& worker) const;

    void pop();

  private:
    static int32_t constexpr Num_entries = 16;

    int32_t index_;

    Interface* stack_;
};

}  // namespace prop
}  // namespace scene

#endif
