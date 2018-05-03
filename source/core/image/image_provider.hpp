#ifndef SU_CORE_IMAGE_PROVIDER_HPP
#define SU_CORE_IMAGE_PROVIDER_HPP

#include "resource/resource_provider.hpp"
#include "encoding/png/png_reader.hpp"
#include "procedural/flakes/flakes_provider.hpp"

namespace image {

class Image;

class Provider final : public resource::Provider<Image> {

public:

	Provider();
	virtual ~Provider() override final;

	virtual std::shared_ptr<Image> load(std::string const& filename,
										memory::Variant_map const& options,
										resource::Manager& manager) override final;

	virtual std::shared_ptr<Image> load(void const* data,
										std::string const& mount_folder,
										memory::Variant_map const& options,
										resource::Manager& manager) override final;

	virtual size_t num_bytes() const override final;

private:

	encoding::png::Reader png_reader_;
	procedural::flakes::Provider flakes_provider_;
};

}

#endif
