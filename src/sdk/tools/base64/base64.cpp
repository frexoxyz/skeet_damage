#include "base64.h"

namespace {
	constexpr char encode_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

	constexpr std::array<std::int8_t, 256> build_decode_table() {
		std::array<std::int8_t, 256> table{};
		for (auto& v : table)
			v = -1;
		for (int i = 0; i < 64; ++i)
			table[static_cast<unsigned char>(encode_table[i])] = static_cast<std::int8_t>(i);
		return table;
	}

	constexpr auto decode_table = build_decode_table();
}

std::string sdk::base64::encode(const std::uint8_t* data, std::size_t size) {
	std::string out;
	out.reserve(((size + 2) / 3) * 4);

	std::size_t i = 0;
	while (i + 3 <= size) {
		const std::uint32_t chunk = (data[i] << 16) | (data[i + 1] << 8) | data[i + 2];
		out += encode_table[(chunk >> 18) & 0x3F];
		out += encode_table[(chunk >> 12) & 0x3F];
		out += encode_table[(chunk >> 6) & 0x3F];
		out += encode_table[chunk & 0x3F];
		i += 3;
	}

	const std::size_t remaining = size - i;
	if (remaining == 1) {
		const std::uint32_t chunk = data[i] << 16;
		out += encode_table[(chunk >> 18) & 0x3F];
		out += encode_table[(chunk >> 12) & 0x3F];
		out += "==";
	}
	else if (remaining == 2) {
		const std::uint32_t chunk = (data[i] << 16) | (data[i + 1] << 8);
		out += encode_table[(chunk >> 18) & 0x3F];
		out += encode_table[(chunk >> 12) & 0x3F];
		out += encode_table[(chunk >> 6) & 0x3F];
		out += '=';
	}

	return out;
}

std::string sdk::base64::encode(const std::string& data) {
	return encode(reinterpret_cast<const std::uint8_t*>(data.data()), data.size());
}

std::vector<std::uint8_t> sdk::base64::decode(const std::string& text) {
	std::vector<std::uint8_t> out;
	out.reserve((text.size() / 4) * 3);

	std::uint32_t buffer = 0;
	int bits = 0;

	for (const char c : text) {
		if (c == '=' || c == '\0')
			break;

		const std::int8_t value = decode_table[static_cast<unsigned char>(c)];
		if (value < 0)
			continue;

		buffer = (buffer << 6) | static_cast<std::uint32_t>(value);
		bits += 6;

		if (bits >= 8) {
			bits -= 8;
			out.push_back(static_cast<std::uint8_t>((buffer >> bits) & 0xFF));
		}
	}

	return out;
}
