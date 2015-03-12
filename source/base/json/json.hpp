#pragma once

#include "math/vector.hpp"
// #include "Math/Matrix.hpp"
// #include "Math/Quaternion.hpp"
#include <rapidjson/document.h>
#include <istream>

namespace json {

bool parse(std::istream& stream, rapidjson::Document& document);

std::string read_error(rapidjson::Document& document, std::istream& stream);

math::float2 read_float2(const rapidjson::Value& value);
math::float3 read_float3(const rapidjson::Value& value);
math::float4 read_float4(const rapidjson::Value& value);

math::uint3  read_uint3(const rapidjson::Value& value);

// math::quaternion read_quaternion(const rapidjson::Value& value);
// math::float3x3   read_rotation_matrix(const rapidjson::Value& value);
// math::quaternion read_local_rotation(const rapidjson::Value& value);

// bool read_bool(const rapidjson::Value& value, const std::string& name, bool default_value);
// std::string read_string(const rapidjson::Value& value, const std::string& name, const std::string& default_value = "");
// uint32_t read_uint(const rapidjson::Value& value, const std::string& name, uint32_t default_value);

}
