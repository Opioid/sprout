#pragma once

#include "resource/resource_provider.hpp"
#include <memory>
#include <string>
#include <vector>

namespace scene { namespace shape {

class Shape;

namespace triangle {

class Provider : public resource::Provider<Shape> {
public:

	Provider(file::System& file_system, thread::Pool& thread_pool);

	enum class BVH_preset {
		Unknown = 0,
		Fast	= 1,
		Slow	= 2
	};

	virtual std::shared_ptr<Shape> load(const std::string& filename, const memory::Variant_map& options) final override;

private:

	std::shared_ptr<Shape> load_morphable_mesh(const std::string& filename,
											   const std::vector<std::string>& morph_targets);
};

}}}
