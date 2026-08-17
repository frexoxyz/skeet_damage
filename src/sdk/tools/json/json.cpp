#include "json.h"
#include <cctype>
#include <cstdlib>
#include <sstream>

sdk::json::value sdk::json::value::make_array() {
	return value(array_t{});
}

sdk::json::value sdk::json::value::make_object() {
	return value(object_t{});
}

bool sdk::json::value::is_null() const {
	return std::holds_alternative<std::monostate>(data);
}

bool sdk::json::value::is_object() const {
	return std::holds_alternative<object_t>(data);
}

bool sdk::json::value::is_array() const {
	return std::holds_alternative<array_t>(data);
}

bool sdk::json::value::as_bool(bool fallback) const {
	return std::holds_alternative<bool>(data) ? std::get<bool>(data) : fallback;
}

double sdk::json::value::as_number(double fallback) const {
	return std::holds_alternative<double>(data) ? std::get<double>(data) : fallback;
}

std::string sdk::json::value::as_string(const std::string& fallback) const {
	return std::holds_alternative<std::string>(data) ? std::get<std::string>(data) : fallback;
}

sdk::json::value::array_t& sdk::json::value::array() {
	if (!is_array())
		data = array_t{};
	return std::get<array_t>(data);
}

const sdk::json::value::array_t* sdk::json::value::array_ptr() const {
	return std::holds_alternative<array_t>(data) ? &std::get<array_t>(data) : nullptr;
}

sdk::json::value::object_t& sdk::json::value::object() {
	if (!is_object())
		data = object_t{};
	return std::get<object_t>(data);
}

const sdk::json::value::object_t* sdk::json::value::object_ptr() const {
	return std::holds_alternative<object_t>(data) ? &std::get<object_t>(data) : nullptr;
}

sdk::json::value& sdk::json::value::operator[](const std::string& key) {
	return object()[key];
}

const sdk::json::value* sdk::json::value::find(const std::string& key) const {
	const auto* obj = object_ptr();
	if (!obj)
		return nullptr;

	const auto it = obj->find(key);
	return it != obj->end() ? &it->second : nullptr;
}

void sdk::json::value::push_back(value v) {
	array().push_back(std::move(v));
}

namespace {
	void dump_string(const std::string& s, std::ostringstream& out) {
		out << '"';
		for (const char c : s) {
			switch (c) {
			case '"': out << "\\\""; break;
			case '\\': out << "\\\\"; break;
			case '\n': out << "\\n"; break;
			case '\r': out << "\\r"; break;
			case '\t': out << "\\t"; break;
			default:
				if (static_cast<unsigned char>(c) < 0x20) {
					char buf[8];
					std::snprintf(buf, sizeof(buf), "\\u%04x", c);
					out << buf;
				}
				else {
					out << c;
				}
			}
		}
		out << '"';
	}

	void dump_number(double v, std::ostringstream& out) {
		if (v == static_cast<double>(static_cast<long long>(v)))
			out << static_cast<long long>(v);
		else
			out << v;
	}

	void dump_value(const sdk::json::value& v, std::ostringstream& out, int depth) {
		const std::string indent(static_cast<std::size_t>(depth) * 4, ' ');
		const std::string indent_in(static_cast<std::size_t>(depth + 1) * 4, ' ');

		if (v.is_null()) {
			out << "null";
		}
		else if (std::holds_alternative<bool>(v.data)) {
			out << (std::get<bool>(v.data) ? "true" : "false");
		}
		else if (std::holds_alternative<double>(v.data)) {
			dump_number(std::get<double>(v.data), out);
		}
		else if (std::holds_alternative<std::string>(v.data)) {
			dump_string(std::get<std::string>(v.data), out);
		}
		else if (const auto* arr = v.array_ptr()) {
			if (arr->empty()) {
				out << "[]";
				return;
			}
			out << "[\n";
			for (std::size_t i = 0; i < arr->size(); ++i) {
				out << indent_in;
				dump_value((*arr)[i], out, depth + 1);
				if (i + 1 < arr->size())
					out << ',';
				out << '\n';
			}
			out << indent << ']';
		}
		else if (const auto* obj = v.object_ptr()) {
			if (obj->empty()) {
				out << "{}";
				return;
			}
			out << "{\n";
			std::size_t i = 0;
			for (const auto& [key, field] : *obj) {
				out << indent_in;
				dump_string(key, out);
				out << ": ";
				dump_value(field, out, depth + 1);
				if (++i < obj->size())
					out << ',';
				out << '\n';
			}
			out << indent << '}';
		}
	}

	class parser_t {
	public:
		explicit parser_t(std::string_view text) : text(text) {}

		std::optional<sdk::json::value> parse() {
			skip_whitespace();
			auto result = parse_value();
			if (!result)
				return std::nullopt;
			skip_whitespace();
			return result;
		}

	private:
		std::string_view text;
		std::size_t pos = 0;

		bool eof() const { return pos >= text.size(); }
		char peek() const { return eof() ? '\0' : text[pos]; }
		char advance() { return eof() ? '\0' : text[pos++]; }

		void skip_whitespace() {
			while (!eof() && std::isspace(static_cast<unsigned char>(peek())))
				++pos;
		}

		bool consume(char c) {
			if (peek() != c)
				return false;
			++pos;
			return true;
		}

		bool consume_literal(std::string_view lit) {
			if (text.substr(pos, lit.size()) != lit)
				return false;
			pos += lit.size();
			return true;
		}

		std::optional<sdk::json::value> parse_value() {
			skip_whitespace();
			switch (peek()) {
			case '{': return parse_object();
			case '[': return parse_array();
			case '"': return parse_string_value();
			case 't':
				if (consume_literal("true")) return sdk::json::value(true);
				return std::nullopt;
			case 'f':
				if (consume_literal("false")) return sdk::json::value(false);
				return std::nullopt;
			case 'n':
				if (consume_literal("null")) return sdk::json::value();
				return std::nullopt;
			default:
				return parse_number();
			}
		}

		std::optional<std::string> parse_raw_string() {
			if (!consume('"'))
				return std::nullopt;

			std::string out;
			while (!eof() && peek() != '"') {
				char c = advance();
				if (c == '\\') {
					const char esc = advance();
					switch (esc) {
					case '"': out += '"'; break;
					case '\\': out += '\\'; break;
					case '/': out += '/'; break;
					case 'n': out += '\n'; break;
					case 'r': out += '\r'; break;
					case 't': out += '\t'; break;
					case 'u': pos += 4; out += '?'; break;
					default: out += esc; break;
					}
				}
				else {
					out += c;
				}
			}

			if (!consume('"'))
				return std::nullopt;

			return out;
		}

		std::optional<sdk::json::value> parse_string_value() {
			auto s = parse_raw_string();
			if (!s)
				return std::nullopt;
			return sdk::json::value(std::move(*s));
		}

		std::optional<sdk::json::value> parse_number() {
			const std::size_t start = pos;
			if (peek() == '-')
				++pos;
			while (!eof() && (std::isdigit(static_cast<unsigned char>(peek())) || peek() == '.' || peek() == 'e' || peek() == 'E' || peek() == '+' || peek() == '-'))
				++pos;

			if (pos == start)
				return std::nullopt;

			const std::string token(text.substr(start, pos - start));
			char* end = nullptr;
			const double v = std::strtod(token.c_str(), &end);
			if (end == token.c_str())
				return std::nullopt;

			return sdk::json::value(v);
		}

		std::optional<sdk::json::value> parse_array() {
			if (!consume('['))
				return std::nullopt;

			auto result = sdk::json::value::make_array();
			skip_whitespace();
			if (consume(']'))
				return result;

			while (true) {
				auto element = parse_value();
				if (!element)
					return std::nullopt;
				result.push_back(std::move(*element));

				skip_whitespace();
				if (consume(','))
					continue;
				if (consume(']'))
					break;
				return std::nullopt;
			}

			return result;
		}

		std::optional<sdk::json::value> parse_object() {
			if (!consume('{'))
				return std::nullopt;

			auto result = sdk::json::value::make_object();
			skip_whitespace();
			if (consume('}'))
				return result;

			while (true) {
				skip_whitespace();
				auto key = parse_raw_string();
				if (!key)
					return std::nullopt;

				skip_whitespace();
				if (!consume(':'))
					return std::nullopt;

				auto field = parse_value();
				if (!field)
					return std::nullopt;

				result.object()[*key] = std::move(*field);

				skip_whitespace();
				if (consume(','))
					continue;
				if (consume('}'))
					break;
				return std::nullopt;
			}

			return result;
		}
	};
}

std::string sdk::json::value::dump() const {
	std::ostringstream out;
	dump_value(*this, out, 0);
	return out.str();
}

std::optional<sdk::json::value> sdk::json::value::parse(std::string_view text) {
	parser_t parser(text);
	return parser.parse();
}
