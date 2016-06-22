#pragma once

namespace resource { class Manager; }

namespace scene { class Scene; }

namespace take { struct Take; }

namespace thread { class Pool; }

namespace controller {

void progressive(const take::Take& take, scene::Scene& scene,
				 resource::Manager& resource_manager, thread::Pool& thread_pool);

}
