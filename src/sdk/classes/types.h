#pragma once

#define INVALID_EHANDLE_INDEX 0xFFFFFFFFu
#define ENT_ENTRY_MASK 0x7FFFu

namespace sdk::types {
	class base_handle {
	public:
		bool is_valid() const noexcept { return index != INVALID_EHANDLE_INDEX; }
		int get_entry_index() const noexcept { return static_cast<int>(index & ENT_ENTRY_MASK); }

	private:
		std::uint32_t index{ INVALID_EHANDLE_INDEX };
	};
}
