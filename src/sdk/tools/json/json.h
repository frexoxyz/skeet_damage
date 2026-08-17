#pragma once
#include <string>
#include <vector>
#include <map>
#include <variant>
#include <optional>
#include <string_view>

namespace sdk::json {
	class value {
	public:
		using array_t = std::vector<value>;
		using object_t = std::map<std::string, value>;
		using storage_t = std::variant<std::monostate, bool, double, std::string, array_t, object_t>;

		value() = default;
		value(bool v) : data(v) {}
		value(int v) : data(static_cast<double>(v)) {}
		value(double v) : data(v) {}
		value(float v) : data(static_cast<double>(v)) {}
		value(const char* v) : data(std::string(v)) {}
		value(std::string v) : data(std::move(v)) {}
		value(array_t v) : data(std::move(v)) {}
		value(object_t v) : data(std::move(v)) {}

		static value make_array();
		static value make_object();

		bool is_null() const;
		bool is_object() const;
		bool is_array() const;

		bool as_bool(bool fallback = false) const;
		double as_number(double fallback = 0.0) const;
		std::string as_string(const std::string& fallback = "") const;

		array_t& array();
		const array_t* array_ptr() const;

		object_t& object();
		const object_t* object_ptr() const;

		value& operator[](const std::string& key);
		const value* find(const std::string& key) const;

		void push_back(value v);

		std::string dump() const;
		static std::optional<value> parse(std::string_view text);

		storage_t data;
	};
}
