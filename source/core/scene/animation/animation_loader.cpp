#include "animation_loader.hpp"
#include "animation.hpp"
#include "base/json/json.hpp"
#include "base/math/vector.inl"
#include "base/math/quaternion.inl"

namespace scene { namespace animation {

void read_morphing(const rapidjson::Value& value, entity::Keyframe::Morphing& morphing);

std::shared_ptr<animation::Animation> load(const rapidjson::Value& animation_value,
										   const math::transformation& default_transformation) {
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
		keyframe.transformation = default_transformation;

		keyframe.morphing.targets[0] = 0;
		keyframe.morphing.targets[1] = 0;
		keyframe.morphing.weight = 0.f;

		for (auto n = k->MemberBegin(); n != k->MemberEnd(); ++n) {
			const std::string node_name = n->name.GetString();
			const rapidjson::Value& node_value = n->value;

			if ("time" == node_name) {
				keyframe.time = json::read_float(node_value);
			} else if ("transformation" == node_name) {
				json::read_transformation(node_value, keyframe.transformation);
			} else if ("morphing" == node_name) {
				read_morphing(node_value, keyframe.morphing);
			}
		}

		animation->push_back(keyframe);
	}

	return animation;
}

void read_morphing(const rapidjson::Value& value, entity::Keyframe::Morphing& morphing) {
	for (auto n = value.MemberBegin(); n != value.MemberEnd(); ++n) {
		const std::string node_name = n->name.GetString();
		const rapidjson::Value& node_value = n->value;

		if ("targets" == node_name) {
			if (node_value.IsArray() && node_value.Size() >= 2) {
				morphing.targets[0] = static_cast<float>(node_value[0].GetDouble());
				morphing.targets[1] = static_cast<float>(node_value[1].GetDouble());
			}
		} else if ("weight" == node_name) {
			morphing.weight = json::read_float(node_value);
		}
	}
}

}}
