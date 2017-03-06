#include "json.hpp"
#include "math/math.hpp"
#include "math/vector3.inl"
#include "math/matrix3x3.inl"
#include "math/quaternion.inl"
//#include "rapidjson/document.h"
#include "rapidjson/istreamwrapper.h"
#include "rapidjson/error/en.h"
#include <sstream>

namespace json {

// get the 0-based line number
size_t calculate_line_number(std::istream& stream, size_t offset, size_t& column) {
	size_t line = 0;
	size_t count = 0;
	size_t local_column = 0;

	stream.clear();
	stream.seekg(0, std::ios_base::beg);

	char c = 0;

	while (stream && count < offset) {
		stream.get(c);

		++count;
		++local_column;

		if ('\n' == c) {
			++line;
			local_column = 0;
		}
	}

	column = local_column;
	return line;
}

std::string read_error(rapidjson::Document& document, std::istream& stream) {
	size_t column;
	size_t line = calculate_line_number(stream, document.GetErrorOffset(), column);

	std::stringstream sstream;
	sstream << rapidjson::GetParseError_En(document.GetParseError());
	// line number is 0-based, so + 1
	sstream << " (line " << line + 1 << ", column " << column + 1 << ")";

	return sstream.str();
}

std::unique_ptr<rapidjson::Document> parse_insitu(char* buffer) {
	std::unique_ptr<rapidjson::Document> document = std::make_unique<rapidjson::Document>();

	document->ParseInsitu(buffer);

	if (document->HasParseError()) {
		throw std::runtime_error(rapidjson::GetParseError_En(document->GetParseError()));
	}

	return document;
}

std::unique_ptr<rapidjson::Document> parse(const std::string& buffer) {
	std::unique_ptr<rapidjson::Document> document = std::make_unique<rapidjson::Document>();

	document->Parse(buffer.c_str());

	if (document->HasParseError()) {
		throw std::runtime_error(rapidjson::GetParseError_En(document->GetParseError()));
	}

	return document;
}

std::unique_ptr<rapidjson::Document> parse(std::istream& stream) {
	rapidjson::IStreamWrapper json_stream(stream);

	std::unique_ptr<rapidjson::Document> document = std::make_unique<rapidjson::Document>();

	document->ParseStream<0, rapidjson::UTF8<>>(json_stream);

	if (document->HasParseError()) {
		throw std::runtime_error(read_error(*document.get(), stream));
	}

	return document;
}

bool read_bool(const rapidjson::Value& value) {
	return value.GetBool();
}

bool read_bool(const rapidjson::Value& value, const std::string& name, bool default_value) {
	const rapidjson::Value::ConstMemberIterator node = value.FindMember(name);
	if (value.MemberEnd() == node) {
		return default_value;
	}

	return node->value.GetBool();
}

float read_float(const rapidjson::Value& value) {
	return value.GetFloat();
}

float read_float(const rapidjson::Value& value, const std::string& name, float default_value) {
	const rapidjson::Value::ConstMemberIterator node = value.FindMember(name);
	if (value.MemberEnd() == node) {
		return default_value;
	}

	return node->value.GetFloat();
}

float2 read_float2(const rapidjson::Value& value) {
	return float2(value[0u].GetFloat(),
				  value[1].GetFloat());
}

float2 read_float2(const rapidjson::Value& value, const std::string& name,
				   float2 default_value) {
	const rapidjson::Value::ConstMemberIterator node = value.FindMember(name);
	if (value.MemberEnd() == node) {
		return default_value;
	}

	return read_float2(node->value);
}

float3 read_float3(const rapidjson::Value& value) {
	return float3(value[0].GetFloat(),
				  value[1].GetFloat(),
				  value[2].GetFloat());
}

float3 read_float3(const rapidjson::Value& value, const std::string& name,
				   float3_p default_value) {
	const rapidjson::Value::ConstMemberIterator node = value.FindMember(name);
	if (value.MemberEnd() == node) {
		return default_value;
	}

	return read_float3(node->value);
}

float4 read_float4(const rapidjson::Value& value) {
	return float4(value[0].GetFloat(),
				  value[1].GetFloat(),
				  value[2].GetFloat(),
				  value[3].GetFloat());
}

int32_t read_int(const rapidjson::Value& value) {
	return value.GetInt();
}

uint32_t read_uint(const rapidjson::Value& value) {
	return value.GetUint();
}

uint32_t read_uint(const rapidjson::Value& value, const std::string& name, uint32_t default_value) {
	const rapidjson::Value::ConstMemberIterator node = value.FindMember(name);
	if (value.MemberEnd() == node) {
		return default_value;
	}

	return node->value.GetUint();
}

int2 read_int2(const rapidjson::Value& value) {
	return int2(value[0].GetInt(), value[1].GetInt());
}

int2 read_int2(const rapidjson::Value& value, const std::string& name,
			   int2 default_value) {
	const rapidjson::Value::ConstMemberIterator node = value.FindMember(name);
	if (value.MemberEnd() == node) {
		return default_value;
	}

	return read_int2(node->value);
}

uint2 read_uint2(const rapidjson::Value& value) {
	return uint2(value[0].GetUint(), value[1].GetUint());
}

uint2 read_uint2(const rapidjson::Value& value, const std::string& name,
				 uint2 default_value) {
	const rapidjson::Value::ConstMemberIterator node = value.FindMember(name);
	if (value.MemberEnd() == node) {
		return default_value;
	}

	return read_uint2(node->value);
}

uint3 read_uint3(const rapidjson::Value& value) {
	return uint3(value[0].GetUint(),
				 value[1].GetUint(),
				 value[2].GetUint());
}

float3x3 create_rotation_matrix(float3_p xyz) {
	float3x3 rot_x;
	math::set_rotation_x(rot_x, math::degrees_to_radians(xyz[0]));

	float3x3 rot_y;
	math::set_rotation_y(rot_y, math::degrees_to_radians(xyz[1]));

	float3x3 rot_z;
	math::set_rotation_z(rot_z, math::degrees_to_radians(xyz[2]));

	return rot_z * rot_x * rot_y;
}

float3x3 read_rotation_matrix(const rapidjson::Value& value) {
	float3 rot = read_float3(value);

	return create_rotation_matrix(rot);
}

math::Quaternion read_local_rotation(const rapidjson::Value& value) {
	return math::quaternion::create(read_rotation_matrix(value));
}

std::string read_string(const rapidjson::Value& value) {
	return std::string(value.GetString(), value.GetStringLength());
}

std::string read_string(const rapidjson::Value& value, const std::string& name,
						const std::string& default_value) {
	const rapidjson::Value::ConstMemberIterator node = value.FindMember(name);
	if (value.MemberEnd() == node) {
		return default_value;
	}

	return std::string(node->value.GetString(), node->value.GetStringLength());
}

void read_transformation(const rapidjson::Value& value, math::Transformation& transformation) {
	for (auto& n : value.GetObject()) {
		const std::string node_name = n.name.GetString();

		if ("position" == node_name) {
			transformation.position = json::read_float3(n.value);
		} else if ("scale" == node_name) {
			transformation.scale = json::read_float3(n.value);
		} else if ("rotation" == node_name) {
			transformation.rotation = json::read_local_rotation(n.value);
		}
	}
}

}
