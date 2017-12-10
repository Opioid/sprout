#ifndef SU_BASE_JSON_JSON_HPP
#define SU_BASE_JSON_JSON_HPP

#include "math/vector4.hpp"
#include "math/matrix3x3.hpp"
#include "math/quaternion.hpp"
#include "math/transformation.hpp"

#include "rapidjson/document.h"

#include <iosfwd>
#include <memory>

namespace json {

std::unique_ptr<rapidjson::Document> parse_insitu(char* buffer);
std::unique_ptr<rapidjson::Document> parse(std::string_view buffer);
std::unique_ptr<rapidjson::Document> parse(std::istream& stream);

bool read_bool(const rapidjson::Value& value);
bool read_bool(const rapidjson::Value& value, std::string_view name, bool default_value);

float read_float(const rapidjson::Value& value);
float read_float(const rapidjson::Value& value, std::string_view name, float default_value);

float2 read_float2(const rapidjson::Value& value);
float2 read_float2(const rapidjson::Value& value, std::string_view name, float2 default_value);

float3 read_float3(const rapidjson::Value& value);
float3 read_float3(const rapidjson::Value& value, std::string_view name,
				   const float3& default_value);

float4 read_float4(const rapidjson::Value& value);

int32_t read_int(const rapidjson::Value& value);

uint32_t read_uint(const rapidjson::Value& value);
uint32_t read_uint(const rapidjson::Value& value, std::string_view name,
				   uint32_t default_value = 0);

int2 read_int2(const rapidjson::Value& value);
int2 read_int2(const rapidjson::Value& value, std::string_view name, int2 default_value);

uint2 read_uint2(const rapidjson::Value& value);
uint2 read_uint2(const rapidjson::Value& value, std::string_view name, uint2 default_value);

uint3 read_uint3(const rapidjson::Value& value);

// math::quaternion read_quaternion(const rapidjson::Value& value);

float3x3 create_rotation_matrix(const float3& xyz);
float3x3 read_rotation_matrix(const rapidjson::Value& value);
math::Quaternion read_local_rotation(const rapidjson::Value& value);

std::string read_string(const rapidjson::Value& value);
std::string read_string(const rapidjson::Value& value, std::string_view name,
						const std::string& default_value = "");

void read_transformation(const rapidjson::Value& value, math::Transformation& transformation);

}

#endif
