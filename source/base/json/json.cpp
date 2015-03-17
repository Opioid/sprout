#include "json.hpp"
#include "json_stream.hpp"
#include "math/math.hpp"
#include "math/vector.inl"
#include "math/matrix.inl"
#include "math/quaternion.inl"
//#include <rapidjson/document.h>
#include <sstream>

namespace json {

std::unique_ptr<rapidjson::Document> parse(std::istream& stream) {
	Read_stream json_stream(stream);

	std::unique_ptr<rapidjson::Document> document = std::make_unique<rapidjson::Document>();

	document->ParseStream<0, rapidjson::UTF8<>>(json_stream);

	if (document->HasParseError()) {
		return nullptr;
	}

	return document;
}

size_t calculate_line_number(std::istream& stream, size_t offset);

std::string read_error(rapidjson::Document& document, std::istream& stream) {
	size_t line = calculate_line_number(stream, document.GetErrorOffset());

	std::stringstream sstream;

	sstream << document.GetParseError();
	// line number is 0-based, so + 1
	sstream << " (line " << line + 1 << ")";

	return sstream.str();
}

// get the 0-based line number
size_t calculate_line_number(std::istream& stream, size_t offset) {
	size_t line = 0;
	size_t count = 0;

	stream.clear();
	stream.seekg(0, std::ios_base::beg);

	char c = 0;

	while (stream && count < offset) {
		stream.get(c);

		++count;

		if ('\n' == c) {
			++line;
		}
	}

	return line;
}

float read_float(const rapidjson::Value& value) {
	return static_cast<float>(value.GetDouble());
}

math::float2 read_float2(const rapidjson::Value& value) {
	return math::float2(static_cast<float>(value[0u].GetDouble()), static_cast<float>(value[1].GetDouble()));
}

math::float3 read_float3(const rapidjson::Value& value) {
	return math::float3(static_cast<float>(value[0u].GetDouble()), static_cast<float>(value[1].GetDouble()),
						static_cast<float>(value[2].GetDouble()));
}

math::float3 read_float3(const rapidjson::Value& value, const std::string& name, const math::float3& default_value) {
	const rapidjson::Value::ConstMemberIterator node = value.FindMember(name.c_str());
	if (value.MemberEnd() == node) {
		return default_value;
	}

	return read_float3(node->value);
}

math::float4 read_float4(const rapidjson::Value& value) {
	return math::float4(static_cast<float>(value[0u].GetDouble()), static_cast<float>(value[1].GetDouble()),
						static_cast<float>(value[2].GetDouble()), static_cast<float>(value[3].GetDouble()));
}

uint32_t read_uint(const rapidjson::Value& value, const std::string& name, uint32_t default_value) {
	const rapidjson::Value::ConstMemberIterator node = value.FindMember(name.c_str());

	if (value.MemberEnd() == node) {
		return default_value;
	}

	return node->value.GetUint();
}

math::uint2 read_uint2(const rapidjson::Value& value) {
	return math::uint2(value[0u].GetUint(), value[1].GetUint());
}

math::uint3 read_uint3(const rapidjson::Value& value) {
	return math::uint3(value[0u].GetUint(), value[1].GetUint(), value[2].GetUint());
}
/*
math::quaternion read_quaternion(const rapidjson::Value& value) {
	 return quaternion(float(value[0u].GetDouble()), float(value[1].GetDouble()), float(value[2].GetDouble()), float(value[3].GetDouble()));
}
*/
math::float3x3 read_rotation_matrix(const rapidjson::Value& value) {
	math::float3 rot = read_float3(value);

	math::float3x3 rot_x;
	set_rotation_x(rot_x, math::degrees_to_radians(rot.x));

	math::float3x3 rot_y;
	set_rotation_y(rot_y, math::degrees_to_radians(rot.y));

	math::float3x3 rot_z;
	set_rotation_z(rot_z, math::degrees_to_radians(rot.z));

    return rot_z * rot_x * rot_y;
}

math::quaternion read_local_rotation(const rapidjson::Value& value) {
	return math::quaternion(read_rotation_matrix(value));
}

/*
bool read_bool(const rapidjson::Value& value, const std::string& name, bool default_value) {
	rapidjson::Value::Member* node = value.FindMember(name.c_str());

	if (!node) {
		return default_value;
	}

	return node->value.GetBool();
}
*/
std::string read_string(const rapidjson::Value& value, const std::string& name, const std::string& default_value) {
	const rapidjson::Value::ConstMemberIterator node = value.FindMember(name.c_str());
	if (value.MemberEnd() == node) {
		return default_value;
	}

	return node->value.GetString();
}

}
