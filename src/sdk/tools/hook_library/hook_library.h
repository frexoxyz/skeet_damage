#pragma once
#include "minhook/include/MinHook.h"

namespace hook_library {
	class hook {
	private:
		void* target = nullptr;
		void* original_pointer = nullptr;
	public:
		hook() = default;
		hook(void* p_function, void* p_detour);

		bool create(void* p_function, void* p_detour);
		bool enable();
		bool disable();
		bool remove();

		void* get_target() const { return target; }

		template <typename T>
		T original() const {
			return reinterpret_cast<T>(original_pointer);
		}
	};

	bool initialize();
	void shut_down();
}
