#pragma once

#include "math/vector.hpp"
#include "math/matrix.hpp"
#include "math/quaternion.hpp"
#include "math/transformation.hpp"
#include <rapidjson/document.h>
#include <istream>
#include <memory>

namespace json {

std::unique_ptr<rapidjson::Document> parse(std::istream& stream);

bool read_bool(const rapidjson::Value& value);
bool read_bool(const rapidjson::Value& value, const std::string& name, bool default_value = false);

float read_float(const rapidjson::Value& value);
float read_float(const rapidjson::Value& value, const std::string& name, float default_value = 0.f);

math::float2 read_float2(const rapidjson::Value& value);

math::float3 read_float3(const rapidjson::Value& value);
math::float3 read_float3(const rapidjson::Value& value, const std::string& name,
						 const math::float3& default_value = math::float3::identity);

math::float4 read_float4(const rapidjson::Value& value);

uint32_t read_uint(const rapidjson::Value& value);
uint32_t read_uint(const rapidjson::Value& value, const std::string& name, uint32_t default_value = 0);

math::int2 read_int2(const rapidjson::Value& value);
math::int2 read_int2(const rapidjson::Value& value, const std::string& name,
					 math::int2 default_value = math::int2::identity);

math::uint2 read_uint2(const rapidjson::Value& value);
math::uint2 read_uint2(const rapidjson::Value& value, const std::string& name,
					   math::uint2 default_value = math::uint2::identity);

math::uint3 read_uint3(const rapidjson::Value& value);

// math::quaternion read_quaternion(const rapidjson::Value& value);
math::float3x3   read_rotation_matrix(const rapidjson::Value& value);
math::quaternion read_local_rotation(const rapidjson::Value& value);

std::string read_string(const rapidjson::Value& value);
std::string read_string(const rapidjson::Value& value, const std::string& name, const std::string& default_value = "");

void read_transformation(const rapidjson::Value& value, math::transformation& transformation);

}
