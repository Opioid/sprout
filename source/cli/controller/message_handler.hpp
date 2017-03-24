#pragma once

#include "server/message_handler.hpp"

namespace rendering { class Driver_progressive; }

namespace resource { class Manager; }

namespace scene {

namespace entity { class Entity; }
namespace material { class Material; }

}

namespace controller {

class Message_handler : public server::Message_handler {

public:

	Message_handler(rendering::Driver_progressive& driver, resource::Manager& resource_manager);

	virtual void handle(const std::string& message) final override;

	virtual std::string introduction() const final override;

	virtual std::string iteration() const final override;

private:

	void handle_entity(scene::entity::Entity* entity, const std::string& value,
					   const std::string& parameters, bool recompile);

	void handle_material(scene::material::Material* material, const std::string& value,
						 const std::string& parameters);

	rendering::Driver_progressive& driver_;
	resource::Manager& resource_manager_;
};

}
