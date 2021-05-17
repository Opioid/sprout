#ifndef SU_CORE_SCENE_PROP_PROP_HPP
#define SU_CORE_SCENE_PROP_PROP_HPP

#include "base/flags/flags.hpp"
#include "base/math/vector.hpp"
#include "resource/resource.hpp"
#include "scene/material/sampler_settings.hpp"

#include <cstddef>

namespace thread {
class Pool;
}

using Threads = thread::Pool;

namespace scene {

class Scene;
class Worker;
struct Ray;

namespace material {
class Material;
}

namespace shape {
class Shape;
struct Intersection;
enum class Interpolation;
}  // namespace shape

namespace prop {

inline uint32_t constexpr Null = 0xFFFFFFFF;

class Prop {
  public:
    using Filter = material::Sampler_settings::Filter;

    Prop();

    ~Prop();

    uint32_t shape() const;

    void configure(uint32_t shape, uint32_t const* materials, Scene const& scene);

    void configure_animated(bool local_animation, Scene const& scene);

    bool has_local_animation() const;

    bool has_no_parent() const;

    void set_has_parent();

    bool visible_in_camera() const;

    bool visible_in_reflection() const;

    bool visible_in_shadow() const;

    bool has_masked_material() const;

    bool has_tinted_shadow() const;

    void set_visible_in_shadow(bool value);

    void set_visibility(bool in_camera, bool in_reflection, bool in_shadow);

    void morph(uint32_t self, Threads& threads, Scene const& scene);

    bool intersect(uint32_t self, Ray& ray, Worker& worker, shape::Interpolation ipo,
                   shape::Intersection& isec) const;

    bool intersect_shadow(uint32_t self, Ray& ray, Worker& worker, shape::Intersection& isec) const;

    bool intersect_p(uint32_t self, Ray const& ray, Worker& worker) const;

    float visibility(uint32_t self, Ray const& ray, Filter filter, Worker& worker) const;

    bool thin_absorption(uint32_t self, Ray const& ray, Filter filter, Worker& worker,
                         float3& ta) const;

  private:
    bool visible(uint32_t ray_depth) const;

    enum class Property {
        Visible_in_camera     = 1 << 0,
        Visible_in_reflection = 1 << 1,
        Visible_in_shadow     = 1 << 2,
        Masked_material       = 1 << 3,
        Tinted_shadow         = 1 << 4,
        Test_AABB             = 1 << 5,
        Has_parent            = 1 << 6,
        Static                = 1 << 7,
        Local_animation       = 1 << 8
    };

    flags::Flags<Property> properties_;

    uint32_t shape_ = resource::Null;
};

struct alignas(8) Prop_topology {
    uint32_t next  = Null;
    uint32_t child = Null;
};

}  // namespace prop

}  // namespace scene

#endif
