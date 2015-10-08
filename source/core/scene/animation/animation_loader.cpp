#include "animation_loader.hpp"
#include "animation.hpp"
#include "base/json/json.hpp"
#include "base/math/vector.inl"
#include "base/math/quaternion.inl"

namespace scene { namespace animation {

std::shared_ptr<animation::Animation> load(const rapidjson::Value& animation_value) {
	const rapidjson::Value::ConstMemberIterator keyframes_node = animation_value.FindMember("keyframes");
	if (animation_value.MemberEnd() == keyframes_node) {
		return nullptr;
	}

	const rapidjson::Value& keyframes_value = keyframes_node->value;

	if (!keyframes_value.IsArray()) {
		return nullptr;
	}

	auto animation = std::make_shared<animation::Animation>();

	animation->init(keyframes_value.Size());

	for (auto k = keyframes_value.Begin(); k != keyframes_value.End(); ++k) {
		entity::Keyframe keyframe;
		keyframe.transformation.position = math::float3::identity;
		keyframe.transformation.scale = math::float3(1.f, 1.f, 1.f);
		keyframe.transformation.rotation = math::quaternion::identity;

		for (auto n = k->MemberBegin(); n != k->MemberEnd(); ++n) {
			const std::string node_name = n->name.GetString();
			const rapidjson::Value& node_value = n->value;

			if ("time" == node_name) {
				keyframe.time = json::read_float(node_value);
			} else if ("transformation" == node_name) {
				json::read_transformation(node_value, keyframe.transformation);
			}
		}

		animation->push_back(keyframe);
	}

	return animation;
}

}}
