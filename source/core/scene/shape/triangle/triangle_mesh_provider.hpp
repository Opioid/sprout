#pragma once

#include "resource/resource_provider.hpp"
#include <string>
#include <vector>

namespace scene { namespace shape {

class Shape;

namespace triangle {

class Provider : public resource::Provider<Shape> {
public:

	Provider();

	virtual std::shared_ptr<Shape> load(const std::string& filename,
										const memory::Variant_map& options,
										resource::Manager& manager) final override;

private:

	std::shared_ptr<Shape> load_morphable_mesh(const std::string& filename,
											   const std::vector<std::string>& morph_targets,
											   resource::Manager& manager);
};

}}}
