#pragma once

#include <map>
#include <string>

namespace memory {

class Variant_map {
public:

	template<typename T>
	bool query(const std::string& key, T& value) const;

	bool query(const std::string& key, bool& value) const;

	bool query(const std::string& key, uint32_t& value) const;

	bool query(const std::string& key, float& value) const;

	template<typename T>
	void insert(const std::string& key, T value);

	void insert(const std::string& key, bool value);

	void insert(const std::string& key, uint32_t value);

	void insert(const std::string& key, float value);

	bool operator<(const Variant_map& other) const;

private:

	struct Variant {

		Variant();
		Variant(bool bool_value);
		Variant(uint32_t uint_value);
		Variant(float float_value);

		bool operator<(const Variant& other) const;

		enum class Type {
			Unknown,
			Bool,
			Uint,
			Float
		};

		Type type;

		union {
			bool	 bool_value;
			uint32_t uint_value;
			float	 float_value;
		};
	};

	std::map<std::string, Variant> map_;
};

}
