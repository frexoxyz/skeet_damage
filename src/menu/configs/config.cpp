#include "config.h"
#include "vars.h"
#include "../../sdk/tools/base64/base64.h"
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <cstring>
#include <cstdint>
#include <windows.h>
#include <shellapi.h>

namespace {
	constexpr int k_config_schema_version = 1;
}

cheat::config::c_var_base::c_var_base(const char* name) : name(name) {
	registry().push_back(this);
}

std::vector<cheat::config::c_var_base*>& cheat::config::c_var_base::registry() {
	static std::vector<c_var_base*> vars;
	return vars;
}

std::string cheat::config::c_registry::directory() {
	char path[MAX_PATH];
	GetCurrentDirectoryA(MAX_PATH, path);
	return std::string(path) + ("\\skeet_damage\\configs");
}



bool cheat::config::c_registry::save_to_file(const std::string& filename) const {
	std::error_code ec;
	std::filesystem::create_directories(directory(), ec);


	auto root = sdk::json::value::make_object();
	root["__schema_version"] = sdk::json::value(static_cast<double>(k_config_schema_version));
	for (const auto* v : c_var_base::registry()) {
		sdk::json::value field;
		v->write(field);
		root[v->name] = field;
	}


	const auto encoded = sdk::base64::encode(root.dump());

	std::ofstream file(directory() + "\\" + filename, std::ios::binary | std::ios::trunc);
	if (!file.is_open())
		return false;

	file << encoded;
	return true;
}

bool cheat::config::c_registry::load_from_file(const std::string& filename) {
	std::ifstream file(directory() + "\\" + filename, std::ios::binary);
	if (!file.is_open())
		return false;

	const std::string encoded((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	file.close();
	if (encoded.empty())
		return false;

	const auto decoded_bytes = sdk::base64::decode(encoded);
	const std::string json_text(decoded_bytes.begin(), decoded_bytes.end());

	const auto parsed = sdk::json::value::parse(json_text);
	if (!parsed || !parsed->is_object())
		return false;

	int schema_version = 0;
	if (const auto* field = parsed->find("__schema_version"))
		schema_version = static_cast<int>(field->as_number(0.0));

	for (auto* v : c_var_base::registry()) {
		if (const auto* field = parsed->find(v->name))
			v->read(*field);
	}


	if (schema_version < k_config_schema_version) {
		save_to_file(filename);
	}

	return true;
}

std::string cheat::config::c_registry::sanitize_name(const std::string& name) {
	std::string out;
	for (char c : name) {
		if (c == '\\' || c == '/' || c == ':' || c == '*' || c == '?' ||
			c == '"' || c == '<' || c == '>' || c == '|')
			continue;
		out += c;
	}
	while (!out.empty() && (out.front() == ' ' || out.front() == '.')) out.erase(out.begin());
	while (!out.empty() && (out.back() == ' ' || out.back() == '.')) out.pop_back();
	return out;
}

std::string cheat::config::c_registry::file_for(const std::string& name) {
	return sanitize_name(name) + (".cfg");
}

std::vector<std::string> cheat::config::c_registry::list_configs() const {
	std::vector<std::string> out;
	std::error_code ec;
	const std::string dir = directory();
	if (!std::filesystem::exists(dir, ec))
		return out;
	for (const auto& entry : std::filesystem::directory_iterator(dir, ec)) {
		if (ec) break;
		if (!entry.is_regular_file(ec)) continue;
		const auto path = entry.path();
		if (path.extension().string() != std::string((".cfg"))) continue;
		out.push_back(path.stem().string());
	}
	std::sort(out.begin(), out.end(), [](const std::string& a, const std::string& b) {
		return _stricmp(a.c_str(), b.c_str()) < 0;
	});
	return out;
}

bool cheat::config::c_registry::create_config(const std::string& name) {
	const std::string clean = sanitize_name(name);
	if (clean.empty())
		return false;

	std::error_code ec;
	std::filesystem::create_directories(directory(), ec);
	if (std::filesystem::exists(directory() + "\\" + file_for(clean), ec))
		return false;

	auto root = sdk::json::value::make_object();
	root["__schema_version"] = sdk::json::value(static_cast<double>(k_config_schema_version));
	for (const auto* v : c_var_base::registry()) {
		sdk::json::value field;
		v->write_default(field);
		root[v->name] = field;
	}
	root["__binds"] = sdk::json::value::make_array();

	const auto encoded = sdk::base64::encode(root.dump());
	std::ofstream file(directory() + "\\" + file_for(clean), std::ios::binary | std::ios::trunc);
	if (!file.is_open())
		return false;
	file << encoded;
	return true;
}

bool cheat::config::c_registry::delete_config(const std::string& name) {
	const std::string clean = sanitize_name(name);
	if (clean.empty())
		return false;

	std::error_code ec;
	return std::filesystem::remove(directory() + "\\" + file_for(clean), ec);
}

bool cheat::config::c_registry::rename_config(const std::string& old_name, const std::string& new_name) {
	const std::string a = sanitize_name(old_name);
	const std::string b = sanitize_name(new_name);
	if (a.empty() || b.empty() || a == b)
		return false;

	std::error_code ec;
	const std::string dst = directory() + "\\" + file_for(b);
	if (std::filesystem::exists(dst, ec))
		return false;

	std::filesystem::rename(directory() + "\\" + file_for(a), dst, ec);
	return !ec;
}

void cheat::config::c_registry::reset_to_defaults() {
	for (auto* v : c_var_base::registry())
		v->reset_to_default();
}

void cheat::config::c_registry::open_folder() const {
	std::error_code ec;
	std::filesystem::create_directories(directory(), ec);
	ShellExecuteA(nullptr, ("open"), directory().c_str(), nullptr, nullptr, SW_SHOWNORMAL);
}
