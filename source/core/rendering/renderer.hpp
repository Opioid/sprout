#pragma once

namespace scene {

class Scene;

}

namespace rendering {

struct Context;

class Renderer {
public:

	void render(const scene::Scene& scene, const Context& context) const;
};

}
