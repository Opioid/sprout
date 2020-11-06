#include "triangle_json_handler.hpp"
#include "base/math/quaternion.inl"
#include "base/math/vector3.inl"

namespace scene::shape::triangle {

Json_handler::Json_handler() {
    clear();
}

Json_handler::Json_handler(Json_handler&& other) {
    parts_.swap(other.parts_);
    triangles_.swap(other.triangles_);
    vertices_.swap(other.vertices_);
    morph_targets_.swap(other.morph_targets_);
}

Json_handler::Json_handler(Json_handler const& other) {
    // Not happy with this, but in practice it makes life easiser with std::function
    // On one hand I don't like the cast, on the other hand the uggliness is explicit now...
    Json_handler& mo = const_cast<Json_handler&>(other);

    parts_.swap(mo.parts_);
    triangles_.swap(mo.triangles_);
    vertices_.swap(mo.vertices_);
    morph_targets_.swap(mo.morph_targets_);
}

void Json_handler::clear(bool read_indices) {
    read_indices_ = read_indices;
    object_level_ = 0;
    top_object_   = Object::Undefined;

    vertices_.clear();
    triangles_.clear();
    parts_.clear();

    expected_number_ = Number::Undefined;
    expected_string_ = String_type::Undefined;
    expected_object_ = Object::Undefined;

    current_vertex_           = 0;
    current_vertex_element_   = 0;
    current_triangle_         = 0;
    current_triangle_element_ = 0;

    has_positions_           = false;
    has_normals_             = false;
    has_tangents_            = false;
    has_texture_coordinates_ = false;
}

void Json_handler::create_part() {
    parts_.emplace_back(Part{0, 0, 3 * uint32_t(triangles_.size())});
}

bool Json_handler::Null() {
    return true;
}

bool Json_handler::Bool(bool /*b*/) {
    return true;
}

bool Json_handler::Int(int i) {
    handle_vertex(float(i));

    return true;
}

bool Json_handler::Uint(unsigned i) {
    switch (expected_number_) {
        case Number::Material_index:
            parts_.back().material_index = i;
            break;
        case Number::Start_index:
            parts_.back().start_index = i;
            break;
        case Number::Num_indices:
            parts_.back().num_indices = i;
            break;
        case Number::Index:
            add_index(i);
            break;
        case Number::Ignore:
            break;
        default:
            handle_vertex(float(i));
            break;
    }

    return true;
}

bool Json_handler::Int64(int64_t /*i*/) {
    return true;
}

bool Json_handler::Uint64(uint64_t /*i*/) {
    return true;
}

bool Json_handler::Double(double d) {
    handle_vertex(float(d));

    return true;
}

bool Json_handler::RawNumber(char const* /*str*/, rapidjson::SizeType /*length*/, bool /*copy*/) {
    return true;
}

bool Json_handler::String(char const* str, rapidjson::SizeType /*length*/, bool /*copy*/) {
    if (String_type::Morph_target == expected_string_) {
        morph_targets_.emplace_back(str);
    }

    return true;
}

bool Json_handler::StartObject() {
    ++object_level_;

    switch (expected_object_) {
        case Object::Part:
            parts_.emplace_back(Part());
            break;
        default:
            break;
    }

    return true;
}

bool Json_handler::Key(char const* str, rapidjson::SizeType /*length*/, bool /*copy*/) {
    std::string_view const name(str);

    if (1 == object_level_) {
        if ("geometry" == name) {
            top_object_ = Object::Geometry;
            return true;
        }

        if ("morph_targets" == name) {
            top_object_      = Object::Morph_targets;
            expected_string_ = String_type::Morph_target;
            return true;
        }
    }

    if (Object::Geometry == top_object_) {
        if ("parts" == name) {
            expected_object_ = Object::Part;
        } else if ("vertices" == name) {
            top_object_      = Object::Vertices;
            expected_object_ = Object::Undefined;

            if (!parts_.empty()) {
                auto& p = parts_.back();

                uint32_t const num_triangles = (p.start_index + p.num_indices) / 3;
                vertices_.reserve(size_t(0.7f * float(num_triangles)));
            }
        } else if ("material_index" == name && Object::Part == expected_object_) {
            expected_number_ = Number::Material_index;
        } else if ("start_index" == name && Object::Part == expected_object_) {
            expected_number_ = Number::Start_index;
        } else if ("num_indices" == name && Object::Part == expected_object_) {
            expected_number_ = Number::Num_indices;
        } else if ("indices" == name) {
            if (read_indices_) {
                expected_number_ = Number::Index;

                if (!parts_.empty()) {
                    auto& p = parts_.back();

                    uint32_t const num_triangles = (p.start_index + p.num_indices) / 3;
                    triangles_.reserve(num_triangles);
                }
            } else {
                expected_number_ = Number::Ignore;
            }
        }

        return true;
    }

    if (Object::Vertices == top_object_) {
        if ("positions" == name) {
            expected_number_        = Number::Position;
            current_vertex_         = 0;
            current_vertex_element_ = 0;
            has_positions_          = true;
        } else if ("texture_coordinates_0" == name) {
            expected_number_         = Number::Texture_coordinate_0;
            current_vertex_          = 0;
            current_vertex_element_  = 0;
            has_texture_coordinates_ = true;
        } else if ("normals" == name) {
            expected_number_        = Number::Normal;
            current_vertex_         = 0;
            current_vertex_element_ = 0;
            has_normals_            = true;
        } else if ("tangents_and_bitangent_signs" == name) {
            expected_number_        = Number::Tangent;
            current_vertex_         = 0;
            current_vertex_element_ = 0;
            has_tangents_           = true;
        } else if ("tangent_space" == name) {
            expected_number_        = Number::Tangent_space;
            current_vertex_         = 0;
            current_vertex_element_ = 0;
            has_normals_            = true;
            has_tangents_           = true;
        }

        return true;
    }

    expected_object_ = Object::Undefined;

    return true;
}

bool Json_handler::EndObject(size_t /*memberCount*/) {
    if (Object::Vertices == top_object_) {
        top_object_ = Object::Geometry;
    }

    --object_level_;
    return true;
}

bool Json_handler::StartArray() {
    return true;
}

bool Json_handler::EndArray(size_t /*elementCount*/) {
    return true;
}

bool Json_handler::has_positions() const {
    return has_positions_;
}

bool Json_handler::has_normals() const {
    return has_normals_;
}

bool Json_handler::has_tangents() const {
    return has_tangents_;
}

bool Json_handler::has_texture_coordinates() const {
    return has_texture_coordinates_;
}

const std::vector<Part>& Json_handler::parts() const {
    return parts_;
}

std::vector<Part>& Json_handler::parts() {
    return parts_;
}

std::vector<Index_triangle> const& Json_handler::triangles() const {
    return triangles_;
}

std::vector<Index_triangle>& Json_handler::triangles() {
    return triangles_;
}

const std::vector<Vertex>& Json_handler::vertices() const {
    return vertices_;
}

std::vector<Vertex>& Json_handler::vertices() {
    return vertices_;
}

const std::vector<std::string>& Json_handler::morph_targets() const {
    return morph_targets_;
}

void Json_handler::add_index(uint32_t i) {
    if (current_triangle_ == triangles_.size()) {
        triangles_.emplace_back();
    }

    triangles_[current_triangle_].i[current_triangle_element_] = i;

    increment_triangle_element();
}

void Json_handler::increment_triangle_element() {
    ++current_triangle_element_;
    if (current_triangle_element_ >= 3) {
        current_triangle_element_ = 0;
        ++current_triangle_;
    }
}

void Json_handler::handle_vertex(float v) {
    switch (expected_number_) {
        case Number::Position:
            add_position(v);
            break;
        case Number::Normal:
            add_normal(v);
            break;
        case Number::Tangent:
            add_tangent(v);
            break;
        case Number::Tangent_space:
            add_tangent_space(v);
            break;
        case Number::Texture_coordinate_0:
            add_texture_coordinate(v);
            break;
        default:
            break;
    }
}

void Json_handler::add_position(float v) {
    if (vertices_.size() == current_vertex_) {
        vertices_.emplace_back();
    }

    vertices_[current_vertex_].p.v[current_vertex_element_] = v;

    increment_vertex_element(3);
}

void Json_handler::add_normal(float v) {
    if (vertices_.size() == current_vertex_) {
        vertices_.emplace_back();
    }

    vertices_[current_vertex_].n.v[current_vertex_element_] = v;

    increment_vertex_element(3);
}

void Json_handler::add_tangent(float v) {
    if (vertices_.size() == current_vertex_) {
        vertices_.emplace_back();
    }

    if (current_vertex_element_ < 3) {
        vertices_[current_vertex_].t.v[current_vertex_element_] = v;
    } else if (current_vertex_element_ == 3) {
        vertices_[current_vertex_].bitangent_sign = v > 0.f ? 0 : 1;

        vertices_[current_vertex_].pad[0] = 0;
        vertices_[current_vertex_].pad[1] = 0;
        vertices_[current_vertex_].pad[2] = 0;
    }

    increment_vertex_element(4);
}

void Json_handler::add_tangent_space(float v) {
    if (vertices_.size() == current_vertex_) {
        vertices_.emplace_back();
    }

    ts_[current_vertex_element_] = v;

    if (current_vertex_element_ == 3) {
        Quaternion ts = ts_;

        bool bts = false;

        if (ts[3] < 0.f) {
            ts[3] = -ts[3];
            bts   = true;
        }

        float3x3 const tbn = quaternion::create_matrix3x3(ts);

        vertices_[current_vertex_].n = packed_float3(tbn.r[2]);
        vertices_[current_vertex_].t = packed_float3(tbn.r[0]);

        vertices_[current_vertex_].bitangent_sign = bts ? 1 : 0;

        vertices_[current_vertex_].pad[0] = 0;
        vertices_[current_vertex_].pad[1] = 0;
        vertices_[current_vertex_].pad[2] = 0;
    }

    increment_vertex_element(4);
}

void Json_handler::add_texture_coordinate(float v) {
    if (vertices_.size() == current_vertex_) {
        vertices_.emplace_back();
    }

    vertices_[current_vertex_].uv.v[current_vertex_element_] = v;

    increment_vertex_element(2);
}

void Json_handler::increment_vertex_element(uint32_t num_elements) {
    ++current_vertex_element_;
    if (current_vertex_element_ >= num_elements) {
        current_vertex_element_ = 0;
        ++current_vertex_;
    }
}

}  // namespace scene::shape::triangle
