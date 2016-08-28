#pragma once

#include "identifiable.hpp"
#include <istream>
#include <memory>

namespace memory { class Variant_map; }

namespace file { class System; }

namespace thread { class Pool; }

namespace resource {

class Manager;

template<typename T>
class Provider : public Identifiable<T> {

public:

	Provider(const std::string& name);
	virtual ~Provider();

	virtual std::shared_ptr<T> load(const std::string& filename,
									const memory::Variant_map& options,
									Manager& manager) = 0;

	virtual std::shared_ptr<T> load(const void* data,
									const std::string& mount_folder,
									const memory::Variant_map& options,
									Manager& manager) = 0;
};

}
