#ifndef SU_BASE_MEMORY_VARIANT_MAP_HPP
#define SU_BASE_MEMORY_VARIANT_MAP_HPP

#include <map>
#include <string>

namespace memory {

class Variant_map {
  public:
    template <typename T>
    bool query(std::string_view key, T& value) const;

    bool query(std::string_view key, bool& value) const;
    bool query(std::string_view key, int32_t& value) const;
    bool query(std::string_view key, uint32_t& value) const;
    bool query(std::string_view key, float& value) const;

    template <typename T>
    void set(std::string const& key, T value);

    void set(std::string const& key, bool value);
    void set(std::string const& key, int32_t value);
    void set(std::string const& key, uint32_t value);
    void set(std::string const& key, float value);

    void inherit(Variant_map const& other, std::string const& key);
    void inherit_except(Variant_map const& other, std::string_view key);

    bool operator<(Variant_map const& other) const;

  private:
    struct Variant {
        Variant(bool bool_value);
        Variant(int32_t int_value);
        Variant(uint32_t uint_value);
        Variant(float float_value);

        bool operator<(Variant const& other) const;

        enum class Type { Undefined, Bool, Int, Uint, Float };

        Type type = Type::Undefined;

        union {
            bool     bool_value;
            int32_t  int_value;
            uint32_t uint_value;
            float    float_value;
        };
    };

    std::map<std::string, Variant, std::less<>> map_;
};

}  // namespace memory

#endif
