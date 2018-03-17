#pragma once

#include <typeindex>

namespace dukglue
{
	namespace detail
	{
		// same as duk_get_type_name, which is private for some reason *shakes fist*
		static const char* get_type_name(duk_int_t type_idx) {
			static const char* names[] = {
				"none",
				"undefined",
				"null",
				"boolean",
				"number",
				"string",
				"object",
				"buffer",
				"pointer",
				"lightfunc"
			};

			if (type_idx >= 0 && type_idx < sizeof(names) / sizeof(names[0]))
				return names[type_idx];
			else
				return "unknown";
		}

		class TypeInfo
		{
		public:
			TypeInfo(duk_uint_t idx) : index_(idx), base_(nullptr) {}
			TypeInfo(const TypeInfo& rhs) : index_(rhs.index_), base_(rhs.base_) {}

			inline void set_base(TypeInfo* base) {
				base_ = base;
			}

			bool can_cast(duk_uint_t class_idx) const {
				if (index_ == class_idx)
					return true;

				if (base_)
					return base_->can_cast(class_idx);

				return false;
			}

			inline bool operator<(const TypeInfo& rhs) const { return index_ < rhs.index_; }
			inline bool operator<=(const TypeInfo& rhs) const { return index_ <= rhs.index_; }
			inline bool operator>(const TypeInfo& rhs) const { return index_ > rhs.index_; }
			inline bool operator>=(const TypeInfo& rhs) const { return index_ >= rhs.index_; }
			inline bool operator==(const TypeInfo& rhs) const { return index_ == rhs.index_; }
			inline bool operator!=(const TypeInfo& rhs) const { return index_ != rhs.index_; }

		private:
			duk_uint_t index_;
			TypeInfo* base_;
		};
	}
}
