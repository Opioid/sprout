#include "animation_loader.hpp"
#include "animation.hpp"
#include "base/json/json.hpp"
#include "base/math/vector.inl"
#include "base/math/quaternion.inl"

namespace scene { namespace animation {

std::shared_ptr<animation::Animation> load_keyframes(
		const json::Value& keyframes_value, const math::transformation& default_transformation);

std::shared_ptr<animation::Animation> load_sequence(
		const json::Value& keyframes_value, const math::transformation& default_transformation);

void read_morphing(const json::Value& value, entity::Keyframe::Morphing& morphing);

std::shared_ptr<animation::Animation> load(const json::Value& animation_value,
										   const math::transformation& default_transformation) {
	for (auto n = animation_value.MemberBegin(); n != animation_value.MemberEnd(); ++n) {
		const std::string node_name = n->name.GetString();
		const rapidjson::Value& node_value = n->value;

		if ("keyframes" == node_name) {
			return load_keyframes(node_value, default_transformation);
		} else if ("morph_sequence" == node_name) {
			return load_sequence(node_value, default_transformation);
		}
	}

	return nullptr;
}

std::shared_ptr<animation::Animation> load_keyframes(
		const json::Value& keyframes_value, const math::transformation& default_transformation) {
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

std::shared_ptr<animation::Animation> load_sequence(
		const json::Value& sequence_value, const math::transformation& default_transformation) {
	uint32_t start_frame = 0;
	uint32_t num_frames = 0;
	uint32_t frames_per_second = 0;

	for (auto n = sequence_value.MemberBegin(); n != sequence_value.MemberEnd(); ++n) {
		const std::string node_name = n->name.GetString();
		const rapidjson::Value& node_value = n->value;

		if ("start_frame" == node_name) {
			start_frame = json::read_uint(node_value);
		} else if ("num_frames" == node_name) {
			num_frames = json::read_uint(node_value);
		} else if ("frames_per_second" == node_name) {
			frames_per_second = json::read_uint(node_value);
		}
	}

	if (!num_frames || !frames_per_second) {
		return nullptr;
	}

	auto animation = std::make_shared<animation::Animation>();

	animation->init(num_frames);

	float time = 0.f;
	float time_increment = 1.f / static_cast<float>(frames_per_second);

	for (uint32_t i = 0; i < num_frames; ++i) {
		entity::Keyframe keyframe;

		keyframe.time = time;

		keyframe.transformation = default_transformation;

		uint32_t target = start_frame + i;
		keyframe.morphing.targets[0] = target;
		keyframe.morphing.targets[1] = target;
		keyframe.morphing.weight = 0.f;

		animation->push_back(keyframe);

		time += time_increment;
	}

	return animation;
}

void read_morphing(const json::Value& value, entity::Keyframe::Morphing& morphing) {
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
