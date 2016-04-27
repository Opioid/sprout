#pragma once

namespace scene { class Scene; }

namespace take { struct Take; }

namespace thread { class Pool; }

namespace controller {

void progressive(const take::Take& take, scene::Scene& scene, thread::Pool& thread_pool);

}
