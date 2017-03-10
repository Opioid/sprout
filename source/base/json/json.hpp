#pragma once

#include "math/vector3.hpp"
#include "math/matrix3x3.hpp"
#include "math/quaternion.hpp"
#include "math/transformation.hpp"

#ifndef RAPIDJSON_HAS_STDSTRING
#	define RAPIDJSON_HAS_STDSTRING 1
#endif
#include "rapidjson/document.h"

#include <iosfwd>
#include <memory>

namespace json {

std::unique_ptr<rapidjson::Document> parse_insitu(char* buffer);
std::unique_ptr<rapidjson::Document> parse(const std::string& buffer);
std::unique_ptr<rapidjson::Document> parse(std::istream& stream);

bool read_bool(const rapidjson::Value& value);
bool read_bool(const rapidjson::Value& value, const std::string& name, bool default_value = false);

float read_float(const rapidjson::Value& value);
float read_float(const rapidjson::Value& value, const std::string& name, float default_value = 0.f);

float2 read_float2(const rapidjson::Value& value);
float2 read_float2(const rapidjson::Value& value, const std::string& name,
				   float2 default_value = float2::identity());

float3 read_float3(const rapidjson::Value& value);
float3 read_float3(const rapidjson::Value& value, const std::string& name,
				   float3_p default_value = float3::identity());

float4 read_float4(const rapidjson::Value& value);

int32_t read_int(const rapidjson::Value& value);

uint32_t read_uint(const rapidjson::Value& value);
uint32_t read_uint(const rapidjson::Value& value, const std::string& name,
				   uint32_t default_value = 0);

int2 read_int2(const rapidjson::Value& value);
int2 read_int2(const rapidjson::Value& value, const std::string& name,
			   int2 default_value = int2::identity());

uint2 read_uint2(const rapidjson::Value& value);
uint2 read_uint2(const rapidjson::Value& value, const std::string& name,
				 uint2 default_value = uint2::identity());

uint3 read_uint3(const rapidjson::Value& value);

// math::quaternion read_quaternion(const rapidjson::Value& value);

float3x3   create_rotation_matrix(float3_p xyz);
float3x3   read_rotation_matrix(const rapidjson::Value& value);
math::Quaternion read_local_rotation(const rapidjson::Value& value);

std::string read_string(const rapidjson::Value& value);
std::string read_string(const rapidjson::Value& value, const std::string& name,
						const std::string& default_value = "");

void read_transformation(const rapidjson::Value& value, math::Transformation& transformation);

}
