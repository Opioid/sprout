#include "triangle_json_handler.hpp"
#include "base/math/vector.inl"

namespace scene { namespace shape { namespace triangle {

Json_handler::Json_handler () {
	clear();
}

void Json_handler::clear() {
	object_level_ = 0;
	top_object_ = Object::Unknown;
	parts_.clear();
	vertices_.clear();
	indices_.clear();
	expected_number_ = Number::Unknown;
	expected_string_ = String_type::Unknown;
	expected_object_ = Object::Unknown;
	current_vertex_ = 0;
	current_vertex_element_ = 0;
	bvh_preset_ = BVH_preset::Unknown,
	has_positions_ = false;
	has_normals_ = false;
	has_tangents_ = false;
	has_texture_coordinates_ = false;
}

void Json_handler::create_part() {
	parts_.push_back(Part{0, 0, static_cast<uint32_t>(indices_.size())});
}

bool Json_handler::Null() {
	return true;
}

bool Json_handler::Bool(bool /*b*/) {
	return true;
}

bool Json_handler::Int(int i) {
	handle_vertex(static_cast<float>(i));

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
		indices_.push_back(i);
		break;
	default:
		handle_vertex(static_cast<float>(i));
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
	handle_vertex(static_cast<float>(d));

	return true;
}

bool Json_handler::String(const char* str, size_t /*length*/, bool /*copy*/) {
	if (String_type::BVH_preset == expected_string_) {
		std::string bvh_preset_value(str);

		if ("fast" == bvh_preset_value) {
			bvh_preset_ = shape::triangle::BVH_preset::Fast;
		} else if ("slow" == bvh_preset_value) {
			bvh_preset_ = shape::triangle::BVH_preset::Slow;
		}
	} else if (String_type::Morph_target == expected_string_) {
		morph_targets_.push_back(str);
	}

	return true;
}

bool Json_handler::StartObject() {
	++object_level_;

	switch (expected_object_) {
	case Object::Part:
		parts_.push_back(Part());
		break;
	default:
		break;
	}

	return true;
}

bool Json_handler::Key(const char* str, size_t /*length*/, bool /*copy*/) {
	std::string name(str);

	if (1 == object_level_) {
		if ("geometry" == name) {
			top_object_ = Object::Geometry;
			return true;
		} else if ("morph_targets" == name) {
			top_object_ = Object::Morph_targets;
			expected_string_ = String_type::Morph_target;
			return true;
		} else if ("bvh_preset" == name) {
			expected_string_ = String_type::BVH_preset;
			return true;
		}
	}

	if (Object::Geometry == top_object_) {
		if ("parts" == name) {
			expected_object_ = Object::Part;
		} else if ("vertices" == name) {
			top_object_ = Object::Vertices;
		} else if ("material_index" == name && Object::Part == expected_object_) {
			expected_number_ = Number::Material_index;
		} else if ("start_index" == name && Object::Part == expected_object_) {
			expected_number_ = Number::Start_index;
		} else if ("num_indices" == name && Object::Part == expected_object_) {
			expected_number_ = Number::Num_indices;
		} else if ("indices" == name) {
			expected_number_ = Number::Index;
		}

		return true;
	} else if (Object::Vertices == top_object_) {
		if ("positions" == name) {
			expected_number_ = Number::Position;
			current_vertex_ = 0;
			current_vertex_element_ = 0;
			has_positions_ = true;
		} else if ("texture_coordinates_0" == name) {
			expected_number_ = Number::Texture_coordinate_0;
			current_vertex_ = 0;
			current_vertex_element_ = 0;
			has_texture_coordinates_ = true;
		} else if ("normals" == name) {
			expected_number_ = Number::Normal;
			current_vertex_ = 0;
			current_vertex_element_ = 0;
			has_normals_ = true;
		} else if ("tangents_and_bitangent_signs" == name) {
			expected_number_ = Number::Tangent;
			current_vertex_ = 0;
			current_vertex_element_ = 0;
			has_tangents_ = true;
		}

		return true;
	}

	return false;
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

BVH_preset Json_handler::bvh_preset() const {
	return bvh_preset_;
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

const std::vector<Json_handler::Part>& Json_handler::parts() const {
	return parts_;
}

const std::vector<uint32_t>& Json_handler::indices() const {
	return indices_;
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
	case Number::Texture_coordinate_0:
		add_texture_coordinate(v);
		break;
	default:
		break;
	}
}

void Json_handler::add_position(float v) {
	if (current_vertex_ == vertices_.size()) {
		vertices_.push_back(Vertex());
	}

	vertices_[current_vertex_].p.v[current_vertex_element_] = v;

	increment_vertex_element(3);
}

void Json_handler::add_normal(float v) {
	if (current_vertex_ == vertices_.size()) {
		vertices_.push_back(Vertex());
	}

	vertices_[current_vertex_].n.v[current_vertex_element_] = v;

	increment_vertex_element(3);
}

void Json_handler::add_tangent(float v) {
	if (current_vertex_ == vertices_.size()) {
		vertices_.push_back(Vertex());
	}

	if (current_vertex_element_ < 3) {
		vertices_[current_vertex_].t.v[current_vertex_element_] = v;
	} else if (current_vertex_element_ == 3) {
		vertices_[current_vertex_].bitangent_sign = v;
	}

	increment_vertex_element(4);
}

void Json_handler::add_texture_coordinate(float v) {
	if (current_vertex_ == vertices_.size()) {
		vertices_.push_back(Vertex());
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

}}}
