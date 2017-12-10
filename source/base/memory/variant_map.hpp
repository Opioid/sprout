#ifndef SU_BASE_MEMORY_VARIANT_MAP_HPP
#define SU_BASE_MEMORY_VARIANT_MAP_HPP

#include <map>
#include <string>

namespace memory {

class Variant_map {

public:

	template<typename T>
	bool query(std::string_view key, T& value) const;

	bool query(std::string_view key, bool& value) const;
	bool query(std::string_view key, int32_t& value) const;
	bool query(std::string_view key, uint32_t& value) const;
	bool query(std::string_view key, float& value) const;

	template<typename T>
	void set(const std::string& key, T value);

	void set(const std::string& key, bool value);
	void set(const std::string& key, int32_t value);
	void set(const std::string& key, uint32_t value);
	void set(const std::string& key, float value);

	void inherit(const Variant_map& other, const std::string& key);
	void inherit_except(const Variant_map& other, const std::string& key);

	bool operator<(const Variant_map& other) const;

private:

	struct Variant {
		Variant() = default;
		Variant(bool bool_value);
		Variant(int32_t int_value);
		Variant(uint32_t uint_value);
		Variant(float float_value);

		bool operator<(const Variant& other) const;

		enum class Type {
			Undefined,
			Bool,
			Int,
			Uint,
			Float
		};

		Type type = Type::Undefined;

		union {
			bool	 bool_value;
			int32_t  int_value;
			uint32_t uint_value;
			float	 float_value;
		};
	};

	std::map<std::string, Variant, std::less<>> map_;
};

}

#endif
