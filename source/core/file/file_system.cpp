#include "file_system.hpp"
#include "file.hpp"
#include "gzip/gzip_read_stream.hpp"
#include <fstream>
#include <sstream>

namespace file {

std::unique_ptr<std::istream> System::read_stream(const std::string& name) const {
	std::string resolved_name;
	return read_stream(name, resolved_name);
}

std::unique_ptr<std::istream> System::read_stream(const std::string& name,
												  std::string& resolved_name) const {
	auto stream = open_read_stream(name, resolved_name);
	if (!stream) {
		throw std::runtime_error("Stream \"" + name + "\" could not be opened");
	}

	const Type type = query_type(*stream);

	if (Type::GZIP == type) {
		return std::make_unique<gzip::Read_stream>(stream);
	}

	return std::unique_ptr<std::istream>(stream);
}

void System::push_mount(const std::string& folder) {
	// We also have to push empty folders, otherwise popping gets complicated

	std::stringstream stream;
	stream << folder;

	if (folder.back() != '/') {
		stream << "/";
	}

	mount_folders_.push_back(stream.str());
}

void System::pop_mount() {
	mount_folders_.pop_back();
}

std::istream* System::open_read_stream(const std::string& name, std::string& resolved_name) const {
	// TODO: Use something like std::filesytem::exists() when it is available

	for (auto& f : mount_folders_) {
		// Ignore empty folders, because this is handled explicitely
		if (f.empty()) {
			continue;
		}

		resolved_name = f + name;

		std::istream* stream = new std::ifstream(resolved_name, std::ios::binary);
		if (*stream) {
			return stream;
		}

		delete stream;
	}

	std::istream* stream = new std::ifstream(name, std::ios::binary);
	if (*stream) {
		resolved_name = name;
		return stream;
	}

	delete stream;
	return nullptr;
}

}
