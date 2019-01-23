#include "json.hpp"
#include "math/math.hpp"
#include "math/matrix3x3.inl"
#include "math/quaternion.inl"
#include "math/vector4.inl"
#include "memory/unique.inl"
//#include "rapidjson/document.h"
#include <sstream>
#include "rapidjson/error/en.h"
#include "rapidjson/istreamwrapper.h"

namespace json {

// get the 0-based line number
struct Error {
    size_t line;
    size_t column;
};

static Error calculate_line_number(std::istream& stream, size_t offset) noexcept {
    size_t line   = 0;
    size_t count  = 0;
    size_t column = 0;

    stream.clear();
    stream.seekg(0, std::ios_base::beg);

    char c = 0;

    while (stream && count < offset) {
        stream.get(c);

        ++count;
        ++column;

        if ('\n' == c) {
            ++line;
            column = 0;
        }
    }

    return {line, column};
}

static std::string read_error(rapidjson::Document const& document, std::istream& stream) noexcept {
    auto const [line, column] = calculate_line_number(stream, document.GetErrorOffset());

    std::stringstream sstream;
    sstream << rapidjson::GetParseError_En(document.GetParseError());
    // line number is 0-based, so + 1
    sstream << " (line " << line + 1 << ", column " << column + 1 << ")";

    return sstream.str();
}

memory::Unique_ptr<rapidjson::Document> parse_insitu(char* buffer) {
    memory::Unique_ptr<rapidjson::Document> document(new rapidjson::Document);

    document->ParseInsitu(buffer);

    if (document->HasParseError()) {
        throw std::runtime_error(rapidjson::GetParseError_En(document->GetParseError()));
    }

    return document;
}

memory::Unique_ptr<rapidjson::Document> parse(std::string_view buffer) {
    memory::Unique_ptr<rapidjson::Document> document(new rapidjson::Document);

    document->Parse(buffer.data());

    if (document->HasParseError()) {
        throw std::runtime_error(rapidjson::GetParseError_En(document->GetParseError()));
    }

    return document;
}

memory::Unique_ptr<rapidjson::Document> parse(std::istream& stream) {
    rapidjson::IStreamWrapper json_stream(stream);

    memory::Unique_ptr<rapidjson::Document> document(new rapidjson::Document);

    document->ParseStream<0, rapidjson::UTF8<>>(json_stream);

    if (document->HasParseError()) {
        throw std::runtime_error(read_error(*document, stream));
    }

    return document;
}

bool read_bool(rapidjson::Value const& value) noexcept {
    return value.GetBool();
}

bool read_bool(rapidjson::Value const& value, std::string_view name, bool default_value) noexcept {
    if (auto const node = value.FindMember(name.data()); value.MemberEnd() != node) {
        return node->value.GetBool();
    }

    return default_value;
}

float read_float(rapidjson::Value const& value) noexcept {
    return value.GetFloat();
}

float read_float(rapidjson::Value const& value, std::string_view name,
                 float default_value) noexcept {
    if (auto const node = value.FindMember(name.data()); value.MemberEnd() != node) {
        return node->value.GetFloat();
    }

    return default_value;
}

double read_double(rapidjson::Value const& value) noexcept {
    return value.GetDouble();
}

float2 read_float2(rapidjson::Value const& value) noexcept {
    return float2(value[0u].GetFloat(), value[1].GetFloat());
}

float2 read_float2(rapidjson::Value const& value, std::string_view name,
                   float2 default_value) noexcept {
    if (auto const node = value.FindMember(name.data()); value.MemberEnd() != node) {
        return read_float2(node->value);
    }

    return default_value;
}

float3 read_float3(rapidjson::Value const& value) noexcept {
    return float3(value[0].GetFloat(), value[1].GetFloat(), value[2].GetFloat());
}

float3 read_float3(rapidjson::Value const& value, std::string_view name,
                   float3 const& default_value) noexcept {
    if (auto const node = value.FindMember(name.data()); value.MemberEnd() != node) {
        return read_float3(node->value);
    }

    return default_value;
}

float4 read_float4(rapidjson::Value const& value) noexcept {
    return float4(value[0].GetFloat(), value[1].GetFloat(), value[2].GetFloat(),
                  value[3].GetFloat());
}

int32_t read_int(rapidjson::Value const& value) noexcept {
    return value.GetInt();
}

uint32_t read_uint(rapidjson::Value const& value) noexcept {
    return value.GetUint();
}

uint32_t read_uint(rapidjson::Value const& value, std::string_view name,
                   uint32_t default_value) noexcept {
    if (auto const node = value.FindMember(name.data()); value.MemberEnd() != node) {
        return node->value.GetUint();
    }

    return default_value;
}

uint64_t read_uint64(rapidjson::Value const& value) noexcept {
    return value.GetUint64();
}

uint64_t read_uint64(rapidjson::Value const& value, std::string_view name,
                     uint64_t default_value) noexcept {
    if (auto const node = value.FindMember(name.data()); value.MemberEnd() != node) {
        return node->value.GetUint64();
    }

    return default_value;
}

int2 read_int2(rapidjson::Value const& value) noexcept {
    return int2(value[0].GetInt(), value[1].GetInt());
}

int2 read_int2(rapidjson::Value const& value, std::string_view name, int2 default_value) noexcept {
    if (auto const node = value.FindMember(name.data()); value.MemberEnd() != node) {
        return read_int2(node->value);
    }

    return default_value;
}

uint2 read_uint2(rapidjson::Value const& value) noexcept {
    return uint2(value[0].GetUint(), value[1].GetUint());
}

uint2 read_uint2(rapidjson::Value const& value, std::string_view name,
                 uint2 default_value) noexcept {
    if (auto const node = value.FindMember(name.data()); value.MemberEnd() != node) {
        return read_uint2(node->value);
    }

    return default_value;
}

int3 read_int3(rapidjson::Value const& value) noexcept {
    return int3(value[0].GetInt(), value[1].GetInt(), value[2].GetInt());
}

int3 read_int3(json::Value const& value, std::string_view name, int3 default_value) noexcept {
    if (auto const node = value.FindMember(name.data()); value.MemberEnd() != node) {
        return read_int3(node->value);
    }

    return default_value;
}

uint3 read_uint3(rapidjson::Value const& value) noexcept {
    return uint3(value[0].GetUint(), value[1].GetUint(), value[2].GetUint());
}

float3x3 create_rotation_matrix(float3 const& xyz) noexcept {
    float3x3 rot_x;
    math::set_rotation_x(rot_x, math::degrees_to_radians(xyz[0]));

    float3x3 rot_y;
    math::set_rotation_y(rot_y, math::degrees_to_radians(xyz[1]));

    float3x3 rot_z;
    math::set_rotation_z(rot_z, math::degrees_to_radians(xyz[2]));

    return rot_z * rot_x * rot_y;
}

float3x3 read_rotation_matrix(rapidjson::Value const& value) noexcept {
    float3 const rot = read_float3(value);

    return create_rotation_matrix(rot);
}

math::Quaternion read_local_rotation(rapidjson::Value const& value) noexcept {
    return math::quaternion::create(read_rotation_matrix(value));
}

std::string read_string(rapidjson::Value const& value) noexcept {
    return std::string(value.GetString(), value.GetStringLength());
}

std::string read_string(rapidjson::Value const& value, std::string_view name,
                        std::string const& default_value) noexcept {
    if (auto const node = value.FindMember(name.data()); value.MemberEnd() != node) {
        return std::string(node->value.GetString(), node->value.GetStringLength());
    }

    return default_value;
}

void read_transformation(rapidjson::Value const& value,
                         math::Transformation&   transformation) noexcept {
    for (auto& n : value.GetObject()) {
        std::string_view const node_name(n.name.GetString(), n.name.GetStringLength());

        if ("position" == node_name) {
            transformation.position = json::read_float3(n.value);
        } else if ("scale" == node_name) {
            transformation.scale = json::read_float3(n.value);
        } else if ("rotation" == node_name) {
            transformation.rotation = json::read_local_rotation(n.value);
        }
    }
}

}  // namespace json

template class memory::Unique_ptr<json::Document>;
