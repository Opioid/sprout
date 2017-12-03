#ifndef SU_CORE_SCENE_VOLUME_BVH_WRAPPER_HPP
#define SU_CORE_SCENE_VOLUME_BVH_WRAPPER_HPP

#include "scene/bvh/scene_bvh_tree.hpp"

namespace scene {

namespace shape { class Node_stack; }

class Worker;
struct Ray;

namespace volume {

class Volume;

class BVH_wrapper {

public:

	bvh::Tree<Volume>& tree();

	const math::AABB& aabb() const;

	const Volume* intersect(scene::Ray& ray, shape::Node_stack& node_stack,
							float& epsilon, bool& inside) const;

private:

	bvh::Tree<Volume> tree_;
};

}}

#endif
