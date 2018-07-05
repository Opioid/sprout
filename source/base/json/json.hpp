#ifndef SU_BASE_JSON_JSON_HPP
#define SU_BASE_JSON_JSON_HPP

#include "json_types.hpp"
#include "math/matrix.hpp"
#include "math/quaternion.hpp"
#include "math/transformation.hpp"
#include "math/vector.hpp"

#include "rapidjson/document.h"

#include <iosfwd>
#include <memory>

namespace json {

std::unique_ptr<json::Document> parse_insitu(char* buffer);
std::unique_ptr<json::Document> parse(std::string_view buffer);
std::unique_ptr<json::Document> parse(std::istream& stream);

bool read_bool(json::Value const& value);
bool read_bool(json::Value const& value, std::string_view name, bool default_value);

float read_float(json::Value const& value);
float read_float(json::Value const& value, std::string_view name, float default_value);

float2 read_float2(json::Value const& value);
float2 read_float2(json::Value const& value, std::string_view name, float2 default_value);

float3 read_float3(json::Value const& value);
float3 read_float3(json::Value const& value, std::string_view name, float3 const& default_value);

float4 read_float4(rapidjson::Value const& value);

int32_t read_int(rapidjson::Value const& value);

uint32_t read_uint(json::Value const& value);
uint32_t read_uint(json::Value const& value, std::string_view name, uint32_t default_value = 0);

int2 read_int2(json::Value const& value);
int2 read_int2(json::Value const& value, std::string_view name, int2 default_value);

uint2 read_uint2(json::Value const& value);
uint2 read_uint2(json::Value const& value, std::string_view name, uint2 default_value);

int3 read_int3(json::Value const& value);
int3 read_int3(json::Value const& value, std::string_view name, int3 default_value);

uint3 read_uint3(json::Value const& value);

// math::quaternion read_quaternion(rapidjson::Value const& value);

float3x3         create_rotation_matrix(float3 const& xyz);
float3x3         read_rotation_matrix(json::Value const& value);
math::Quaternion read_local_rotation(json::Value const& value);

std::string read_string(json::Value const& value);
std::string read_string(json::Value const& value, std::string_view name,
                        std::string const& default_value = "");

void read_transformation(json::Value const& value, math::Transformation& transformation);

}  // namespace json

#endif
