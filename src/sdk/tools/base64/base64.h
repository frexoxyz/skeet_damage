#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace sdk::base64 {
	std::string encode(const std::uint8_t* data, std::size_t size);
	std::string encode(const std::string& data);
	std::vector<std::uint8_t> decode(const std::string& text);
}
