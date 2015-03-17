#pragma once

#include "math/vector.hpp"
#include "math/matrix.hpp"
#include "math/quaternion.hpp"
#include <rapidjson/document.h>
#include <istream>
#include <memory>

namespace json {

std::unique_ptr<rapidjson::Document> parse(std::istream& stream);

std::string read_error(rapidjson::Document& document, std::istream& stream);

float        read_float(const rapidjson::Value& value);
math::float2 read_float2(const rapidjson::Value& value);

math::float3 read_float3(const rapidjson::Value& value);
math::float3 read_float3(const rapidjson::Value& value, const std::string& name, const math::float3& default_value = math::float3::identity);

math::float4 read_float4(const rapidjson::Value& value);

uint32_t read_uint(const rapidjson::Value& value, const std::string& name, uint32_t default_value = 0);

math::uint2  read_uint2(const rapidjson::Value& value);
math::uint3  read_uint3(const rapidjson::Value& value);

// math::quaternion read_quaternion(const rapidjson::Value& value);
math::float3x3   read_rotation_matrix(const rapidjson::Value& value);
math::quaternion read_local_rotation(const rapidjson::Value& value);

// bool read_bool(const rapidjson::Value& value, const std::string& name, bool default_value);
std::string read_string(const rapidjson::Value& value, const std::string& name, const std::string& default_value = "");


}
