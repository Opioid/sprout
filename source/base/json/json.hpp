#ifndef SU_BASE_JSON_JSON_HPP
#define SU_BASE_JSON_JSON_HPP

#include <iosfwd>
#include "json_types.hpp"
#include "math/matrix.hpp"
#include "math/quaternion.hpp"
#include "math/transformation.hpp"
#include "math/vector.hpp"
#include "memory/unique.hpp"
#include "rapidjson/document.h"

namespace json {

using Document_ptr = memory::Unique_ptr<json::Document>;

Document_ptr parse_insitu(char* buffer, std::string& error) noexcept;
Document_ptr parse(std::string_view buffer, std::string& error) noexcept;
Document_ptr parse(std::istream& stream, std::string& error) noexcept;

bool read_bool(json::Value const& value) noexcept;
bool read_bool(json::Value const& value, std::string_view name, bool default_value) noexcept;

float read_float(json::Value const& value) noexcept;
float read_float(json::Value const& value, std::string_view name, float default_value) noexcept;

double read_double(json::Value const& value) noexcept;

float2 read_float2(json::Value const& value) noexcept;
float2 read_float2(json::Value const& value, std::string_view name, float2 default_value) noexcept;

float3 read_float3(json::Value const& value) noexcept;
float3 read_float3(json::Value const& value, std::string_view name,
                   float3 const& default_value) noexcept;

float4 read_float4(rapidjson::Value const& value) noexcept;

int32_t read_int(rapidjson::Value const& value) noexcept;

uint32_t read_uint(json::Value const& value) noexcept;
uint32_t read_uint(json::Value const& value, std::string_view name,
                   uint32_t default_value = 0) noexcept;

uint64_t read_uint64(json::Value const& value) noexcept;
uint64_t read_uint64(json::Value const& value, std::string_view name,
                     uint64_t default_value = 0) noexcept;

int2 read_int2(json::Value const& value) noexcept;
int2 read_int2(json::Value const& value, std::string_view name, int2 default_value) noexcept;

uint2 read_uint2(json::Value const& value) noexcept;
uint2 read_uint2(json::Value const& value, std::string_view name, uint2 default_value) noexcept;

int3 read_int3(json::Value const& value) noexcept;
int3 read_int3(json::Value const& value, std::string_view name, int3 default_value) noexcept;

uint3 read_uint3(json::Value const& value) noexcept;

// math::quaternion read_quaternion(rapidjson::Value const& value);

float3x3 create_rotation_matrix(float3 const& xyz) noexcept;

float3x3 read_rotation_matrix(json::Value const& value) noexcept;

math::Quaternion read_local_rotation(json::Value const& value) noexcept;

std::string read_string(json::Value const& value) noexcept;
std::string read_string(json::Value const& value, std::string_view name,
                        std::string const& default_value = "") noexcept;

void read_transformation(json::Value const& value, math::Transformation& transformation) noexcept;

}  // namespace json

extern template class memory::Unique_ptr<json::Document>;

#endif
