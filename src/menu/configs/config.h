#pragma once
#include "../../sdk/tools/json/json.h"
#include <type_traits>
#include <vector>

namespace cheat::config {
	class c_var_base {
	public:
		explicit c_var_base(const char* name);
		virtual ~c_var_base() = default;

		virtual void write(sdk::json::value& out) const = 0;
		virtual void write_default(sdk::json::value& out) const = 0;
		virtual void read(const sdk::json::value& in) = 0;
		virtual void reset_to_default() = 0;
		virtual const void* raw_ptr() const = 0;

		std::string name;

		static std::vector<c_var_base*>& registry();
	};

	template <typename T>
	class c_var : public c_var_base {
	public:
		c_var(const char* name, T default_value = T{})
			: c_var_base(name), value(default_value), default_value(default_value) {
		}

		void reset_to_default() override { value = default_value; }
		const void* raw_ptr() const override { return &value; }

		operator const T& () const { return value; }
		T& get() { return value; }
		const T& get() const { return value; }
		c_var& operator=(const T& v) { value = v; return *this; }

		static void write_value(sdk::json::value& out, const T& value) {
			if constexpr (std::is_same_v<T, ImColor>) {
				auto arr = sdk::json::value::make_array();
				arr.push_back(value.Value.x);
				arr.push_back(value.Value.y);
				arr.push_back(value.Value.z);
				arr.push_back(value.Value.w);
				out = arr;
			}
			else if constexpr (std::is_same_v<T, ImVec2>) {
				auto arr = sdk::json::value::make_array();
				arr.push_back(value.x);
				arr.push_back(value.y);
				out = arr;
			}
			else if constexpr (std::is_same_v<T, std::string>) {
				out = sdk::json::value(value);
			}
			else if constexpr (std::is_same_v<T, bool>) {
				out = sdk::json::value(value);
			}
			else if constexpr (std::is_same_v<T, std::vector<bool>>) {
				auto arr = sdk::json::value::make_array();
				for (const bool flag : value)
					arr.push_back(sdk::json::value(flag));
				out = arr;
			}
			else if constexpr (std::is_same_v<T, std::vector<int>>) {
				auto arr = sdk::json::value::make_array();
				for (const int key : value) arr.push_back(sdk::json::value(static_cast<double>(key)));
				out = arr;
			}
			else {
				out = sdk::json::value(static_cast<double>(value));
			}
		}

		void write(sdk::json::value& out) const override { write_value(out, value); }
		void write_default(sdk::json::value& out) const override { write_value(out, default_value); }

		void read(const sdk::json::value& in) override {
			if constexpr (std::is_same_v<T, ImColor>) {
				if (const auto* arr = in.array_ptr(); arr && arr->size() == 4)
					value = ImColor(static_cast<float>((*arr)[0].as_number()), static_cast<float>((*arr)[1].as_number()), static_cast<float>((*arr)[2].as_number()), static_cast<float>((*arr)[3].as_number()));
			}
			else if constexpr (std::is_same_v<T, ImVec2>) {
				if (const auto* arr = in.array_ptr(); arr && arr->size() == 2)
					value = ImVec2(static_cast<float>((*arr)[0].as_number()), static_cast<float>((*arr)[1].as_number()));
			}
			else if constexpr (std::is_same_v<T, std::string>) {
				value = in.as_string(value);
			}
			else if constexpr (std::is_same_v<T, bool>) {
				value = in.as_bool(value);
			}
			else if constexpr (std::is_same_v<T, std::vector<bool>>) {
				if (const auto* arr = in.array_ptr()) {
					for (std::size_t index = 0; index < value.size() &&
						index < arr->size(); ++index)
						value[index] = (*arr)[index].as_bool(value[index]);
				}
			}
			else if constexpr (std::is_same_v<T, std::vector<int>>) {
				if (const auto* arr = in.array_ptr()) {
					value.clear();
					for (const auto& item : *arr) value.push_back(static_cast<int>(item.as_number()));
				}
			}
			else {
				value = static_cast<T>(in.as_number(static_cast<double>(value)));
			}
		}

		T value;
		T default_value;
	};

	class c_registry {
	public:
		static std::string directory();

		bool save_to_file(const std::string& filename = "config.cfg") const;
		bool load_from_file(const std::string& filename = "config.cfg");

		std::vector<std::string> list_configs() const;
		bool create_config(const std::string& name);
		bool delete_config(const std::string& name);
		bool rename_config(const std::string& old_name, const std::string& new_name);
		void reset_to_defaults();
		void open_folder() const;

		static std::string sanitize_name(const std::string& name);
		static std::string file_for(const std::string& name);
	} inline g_registry;
}
