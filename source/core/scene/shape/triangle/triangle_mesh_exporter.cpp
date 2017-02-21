#include "triangle_mesh_exporter.hpp"
#include "triangle_json_handler.hpp"
#include "base/math/vector.inl"
#include <fstream>
#include <iostream>

namespace scene { namespace shape { namespace triangle {

std::string extract_filename(const std::string& filename) {
	size_t i = filename.find_last_of('/') + 1;
	return filename.substr(i, filename.find_first_of('.') - i);
}

void newline(std::ofstream& stream, uint32_t num_tabs) {
	stream << std::endl;

	for (uint32_t i = 0; i < num_tabs; ++i) {
		stream << '\t';
	}
}

void Exporter::write(const std::string& filename, const Json_handler& handler) {
	std::string out_name = extract_filename(filename) + ".json";

	std::cout << "Export " << out_name << std::endl;

	std::ofstream stream(out_name);

	if (!stream) {
		return;
	}

	stream << "{";

	newline(stream, 1);

	stream << "\"geometry\":{";

	newline(stream, 2);

	stream << "\"parts\":[";

	const auto& parts = handler.parts();

	for (size_t i = 0, len = parts.size(); i < len; ++i) {
		newline(stream, 3);

		const auto& p = parts[i];
		stream << "{";
		stream << "\"material_index\":" << p.material_index << ",";
		stream << "\"start_index\":" << p.start_index << ",";
		stream << "\"num_indices\":" << p.num_indices;
		stream << "}";

		if (i < len - 1) {
			stream << ",";
		}
	}

	// close parts
	newline(stream, 2);
	stream << "]";

	// close geometry
	newline(stream, 1);
	stream << "}";

	// close start
	newline(stream, 0);
	stream << "}";

}

}}}
