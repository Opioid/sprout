#pragma once

#include "resource/resource_provider.hpp"
#include "substitute/substitute.hpp"
#include "base/json/rapidjson_types.hpp"

namespace scene { namespace material {

class IMaterial;

namespace substitute {

class Sample;

}

class Provider : public resource::Provider<IMaterial> {
public:

	Provider(uint32_t num_workers);

	virtual std::shared_ptr<IMaterial> load(const std::string& filename);

private:

	std::shared_ptr<IMaterial> load_substitute(const rapidjson::Value& substitute_value);

	Sample_cache<substitute::Sample> substitute_cache_;
};

}}
