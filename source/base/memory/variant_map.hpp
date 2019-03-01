#ifndef SU_BASE_MEMORY_VARIANT_MAP_HPP
#define SU_BASE_MEMORY_VARIANT_MAP_HPP

#include <map>
#include <string>

namespace memory {

class Variant_map {
  public:
    template <typename T>
    bool query(std::string_view key, T& value) const noexcept;

    bool query(std::string_view key, bool& value) const noexcept;
    bool query(std::string_view key, int32_t& value) const noexcept;
    bool query(std::string_view key, uint32_t& value) const noexcept;
    bool query(std::string_view key, float& value) const noexcept;

    template <typename T>
    void set(std::string const& key, T value) noexcept;

    void set(std::string const& key, bool value) noexcept;
    void set(std::string const& key, int32_t value) noexcept;
    void set(std::string const& key, uint32_t value) noexcept;
    void set(std::string const& key, float value) noexcept;

    void inherit(Variant_map const& other, std::string const& key) noexcept;
    void inherit_except(Variant_map const& other, std::string_view key) noexcept;

    bool operator<(Variant_map const& other) const noexcept;

  private:
    struct Variant {
        Variant(bool bool_value) noexcept;
        Variant(int32_t int_value) noexcept;
        Variant(uint32_t uint_value) noexcept;
        Variant(float float_value) noexcept;

        bool operator<(Variant const& other) const noexcept;

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
