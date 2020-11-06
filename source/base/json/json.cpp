#include "json.hpp"
#include "math/math.hpp"
#include "math/matrix3x3.inl"
#include "math/matrix4x4.inl"
#include "math/quaternion.inl"
#include "math/vector4.inl"
#include "memory/buffer.hpp"
#include "rapidjson/error/en.h"
#include "rapidjson/istreamwrapper.h"

#include <sstream>

namespace json {

std::string read_error(rapidjson::Document const& document) {
    return rapidjson::GetParseError_En(document.GetParseError());
}

// get the 0-based line number
struct Error {
    size_t line;
    size_t column;
};

static Error calculate_line_number(std::istream& stream, size_t offset) {
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

std::string read_error(rapidjson::Document const& document, std::istream& stream) {
    auto const [line, column] = calculate_line_number(stream, document.GetErrorOffset());

    std::ostringstream sstream;
    sstream << rapidjson::GetParseError_En(document.GetParseError());
    // line number is 0-based, so + 1
    sstream << " (line " << line + 1 << ", column " << column + 1 << ")";

    return sstream.str();
}

rapidjson::Document parse(std::istream& stream, std::string& error) {
    static size_t constexpr Buffer_size = 8192;

    memory::Buffer<char> buffer(Buffer_size);

    rapidjson::IStreamWrapper json_stream(stream, buffer.data(), Buffer_size);

    rapidjson::Document document;

    document.ParseStream<0, rapidjson::UTF8<>>(json_stream);

    if (document.HasParseError()) {
        error = read_error(document, stream);
    }

    return document;
}

bool read_bool(rapidjson::Value const& value) {
    return value.GetBool();
}

bool read_bool(rapidjson::Value const& value, std::string_view name, bool default_value) {
    if (auto const node = value.FindMember(name.data()); value.MemberEnd() != node) {
        return node->value.GetBool();
    }

    return default_value;
}

float read_float(rapidjson::Value const& value) {
    return value.GetFloat();
}

float read_float(rapidjson::Value const& value, std::string_view name, float default_value) {
    if (auto const node = value.FindMember(name.data()); value.MemberEnd() != node) {
        return node->value.GetFloat();
    }

    return default_value;
}

double read_double(rapidjson::Value const& value) {
    return value.GetDouble();
}

float2 read_float2(rapidjson::Value const& value) {
    return float2(value[0u].GetFloat(), value[1].GetFloat());
}

float2 read_float2(rapidjson::Value const& value, std::string_view name, float2 default_value) {
    if (auto const node = value.FindMember(name.data()); value.MemberEnd() != node) {
        return read_float2(node->value);
    }

    return default_value;
}

float3 read_float3(rapidjson::Value const& value) {
    return float3(value[0].GetFloat(), value[1].GetFloat(), value[2].GetFloat());
}

float3 read_float3(rapidjson::Value const& value, std::string_view name,
                   float3 const& default_value) {
    if (auto const node = value.FindMember(name.data()); value.MemberEnd() != node) {
        return read_float3(node->value);
    }

    return default_value;
}

float4 read_float4(rapidjson::Value const& value) {
    return float4(value[0].GetFloat(), value[1].GetFloat(), value[2].GetFloat(),
                  value[3].GetFloat());
}

int32_t read_int(rapidjson::Value const& value) {
    return value.GetInt();
}

uint32_t read_uint(rapidjson::Value const& value) {
    return value.GetUint();
}

uint32_t read_uint(rapidjson::Value const& value, std::string_view name, uint32_t default_value) {
    if (auto const node = value.FindMember(name.data()); value.MemberEnd() != node) {
        return node->value.GetUint();
    }

    return default_value;
}

uint64_t read_uint64(rapidjson::Value const& value) {
    return value.GetUint64();
}

uint64_t read_uint64(rapidjson::Value const& value, std::string_view name, uint64_t default_value) {
    if (auto const node = value.FindMember(name.data()); value.MemberEnd() != node) {
        return node->value.GetUint64();
    }

    return default_value;
}

int2 read_int2(rapidjson::Value const& value) {
    return int2(value[0].GetInt(), value[1].GetInt());
}

int2 read_int2(rapidjson::Value const& value, std::string_view name, int2 default_value) {
    if (auto const node = value.FindMember(name.data()); value.MemberEnd() != node) {
        return read_int2(node->value);
    }

    return default_value;
}

uint2 read_uint2(rapidjson::Value const& value) {
    return uint2(value[0].GetUint(), value[1].GetUint());
}

uint2 read_uint2(rapidjson::Value const& value, std::string_view name, uint2 default_value) {
    if (auto const node = value.FindMember(name.data()); value.MemberEnd() != node) {
        return read_uint2(node->value);
    }

    return default_value;
}

int3 read_int3(rapidjson::Value const& value) {
    return int3(value[0].GetInt(), value[1].GetInt(), value[2].GetInt());
}

int3 read_int3(json::Value const& value, std::string_view name, int3 default_value) {
    if (auto const node = value.FindMember(name.data()); value.MemberEnd() != node) {
        return read_int3(node->value);
    }

    return default_value;
}

uint3 read_uint3(rapidjson::Value const& value) {
    return uint3(value[0].GetUint(), value[1].GetUint(), value[2].GetUint());
}

int4 read_int4(rapidjson::Value const& value) {
    return int4(value[0].GetInt(), value[1].GetInt(), value[2].GetInt(), value[3].GetInt());
}

int4 read_int4(json::Value const& value, std::string_view name, int4 default_value) {
    if (auto const node = value.FindMember(name.data()); value.MemberEnd() != node) {
        return read_int4(node->value);
    }

    return default_value;
}

float3x3 create_rotation_matrix(float3 const& xyz) {
    float3x3 rot_x;
    set_rotation_x(rot_x, degrees_to_radians(xyz[0]));

    float3x3 rot_y;
    set_rotation_y(rot_y, degrees_to_radians(xyz[1]));

    float3x3 rot_z;
    set_rotation_z(rot_z, degrees_to_radians(xyz[2]));

    return rot_z * rot_x * rot_y;
}

float3x3 read_rotation_matrix(rapidjson::Value const& value) {
    float3 const rot = read_float3(value);

    return create_rotation_matrix(rot);
}

math::Quaternion read_local_rotation(rapidjson::Value const& value) {
    return math::quaternion::create(read_rotation_matrix(value));
}

std::string read_string(rapidjson::Value const& value) {
    return std::string(value.GetString(), value.GetStringLength());
}

std::string read_string(rapidjson::Value const& value, std::string_view name,
                        std::string const& default_value) {
    if (auto const node = value.FindMember(name.data()); value.MemberEnd() != node) {
        return std::string(node->value.GetString(), node->value.GetStringLength());
    }

    return default_value;
}

void read_transformation(rapidjson::Value const& value, math::Transformation& trafo) {
    if (value.IsArray()) {
        float4x4 const m(
            value[0].GetFloat(), value[1].GetFloat(), value[2].GetFloat(), value[3].GetFloat(),
            value[4].GetFloat(), value[5].GetFloat(), value[6].GetFloat(), value[7].GetFloat(),
            value[8].GetFloat(), value[9].GetFloat(), value[10].GetFloat(), value[11].GetFloat(),
            value[12].GetFloat(), value[13].GetFloat(), value[14].GetFloat(), value[15].GetFloat());

        float3x3 r;

        decompose(m, r, trafo.scale, trafo.position);

        trafo.rotation = quaternion::create(r);
    } else {
        float3 up(0.f, 1.f, 0.f);
        float3 look_at(0.f, 0.f, 1.f);

        bool look = false;

        for (auto& n : value.GetObject()) {
            std::string_view const node_name(n.name.GetString(), n.name.GetStringLength());

            if ("position" == node_name) {
                trafo.position = json::read_float3(n.value);
            } else if ("scale" == node_name) {
                trafo.scale = json::read_float3(n.value);
            } else if ("rotation" == node_name) {
                trafo.rotation = json::read_local_rotation(n.value);
            } else if ("up" == node_name) {
                up = json::read_float3(n.value);
            } else if ("look_at" == node_name) {
                look_at = json::read_float3(n.value);
                look    = true;
            }
        }

        if (look) {
            float3 const dir   = normalize(look_at - trafo.position);
            float3 const right = -cross(dir, up);

            float3x3 const r(right, up, dir);

            trafo.rotation = quaternion::create(r);
        }
    }
}

}  // namespace json
