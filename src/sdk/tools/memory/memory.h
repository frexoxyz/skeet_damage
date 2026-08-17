#pragma once

namespace memory
{
	struct address_t
	{
		std::uintptr_t pointer;

		inline address_t() : pointer(0) {}
		inline address_t(const std::uintptr_t& ptr) : pointer(ptr) {}
		inline address_t(const void* ptr) : pointer((std::uintptr_t)ptr) {}

		template <typename T = void*>
		inline T get()
		{
			if (pointer == 0)
				return {};

			return (T)pointer;
		}

		template <typename T = address_t>
		inline T resolve(int prefix = 3, int total_len = 7) const
		{
			if (!pointer )
				return 0;

			std::int32_t rva = *reinterpret_cast<std::int32_t*>(pointer + prefix);
			std::uint64_t rip = pointer + total_len;

			return reinterpret_cast<T>(rva + rip);
		}

		inline bool is_valid()
		{
			MEMORY_BASIC_INFORMATION mbi;

			if (!pointer)
				return false;

			if (!VirtualQuery((const void*)pointer, &mbi, sizeof(mbi)))
				return false;

			if ((mbi.Protect & PAGE_NOACCESS) || (mbi.Protect & PAGE_GUARD))
				return false;

			return true;
		}
	};

	address_t find_pattern(HMODULE module, const char* pat);
	address_t find_interface(HMODULE module, const char* inter);
	address_t find_virtual(address_t ptr, const int& idx);
	HMODULE get_module(const char* name);

	template <typename type_t, typename... Args>
	inline type_t call_virtual(void* class_, unsigned int index, Args... args) {
		return memory::find_virtual(memory::address_t(class_), index).get<type_t(__thiscall*)(void*, Args...)>()(class_, args...);
	}
}
#define VFUNC(_class, ret_type, index, ...) memory::call_virtual<ret_type>(_class, index, __VA_ARGS__)

