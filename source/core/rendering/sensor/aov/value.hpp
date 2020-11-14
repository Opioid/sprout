#ifndef SU_CORE_RENDERING_SENSOR_AOV_VALUE_HPP
#define SU_CORE_RENDERING_SENSOR_AOV_VALUE_HPP

#include "base/math/vector3.hpp"
#include "base/math/vector4.hpp"
#include "property.hpp"

namespace rendering::sensor::aov {

class Value {
  public:
    Value();

    ~Value();

    static uint32_t constexpr Max_slots = 7;

    struct Mapping {
        uint8_t m[Max_slots];
    };

    void init(Mapping mapping, uint32_t num_slots);

    uint32_t num_slots() const;

    bool active(Property aov) const;

    void clear();

    void insert(float3 const& v, Property aov);

    struct Result {
        float3   v;
        Property p;
    };

    float3 value(uint32_t id) const;

    bool accumulating(uint32_t id) const;

  private:
    Mapping mapping_;

    uint8_t num_slots_;

    struct alignas(16) Slot {
        float v[3];
        bool  accumulating;
    };

    Slot* slots_;
};

class Value_pool {
  public:
    Value_pool();

    ~Value_pool();

    void configure(uint32_t num_slots, Property const* properties);

    void init(uint32_t num_values);

    Value::Mapping const& mapping() const;

    uint32_t num_slots() const;

    Property property(uint32_t slot) const;

    Value* get(uint32_t id) const;

  private:
    Value::Mapping mapping_;

    uint8_t num_slots_;

    Property* properties_;

    Value* values_;
};

}  // namespace rendering::sensor::aov

#endif
