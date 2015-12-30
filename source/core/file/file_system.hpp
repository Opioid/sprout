#pragma once

#include <istream>
#include <memory>
#include <string>
#include <vector>

namespace file {

class System {
public:

	std::unique_ptr<std::istream> read_stream(const std::string& name) const;

	void push_mount(const std::string& folder);

private:

	std::unique_ptr<std::istream> open_read_stream(const std::string& name) const;

	std::vector<std::string> mount_folders_;
};

}
