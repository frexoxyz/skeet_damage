#include "memory.h"

#include <cstring>

memory::address_t memory::find_pattern(HMODULE module, const char* pat)
{
    static auto pattern_to_byte = [](const char* pattern) {
        auto bytes = std::vector<int>{};
        auto start = const_cast<char*>(pattern);
        auto end = const_cast<char*>(pattern) + strlen(pattern);

        for (auto current = start; current < end; ++current) {
            if (*current == '?') {
                ++current;
                if (*current == '?')
                    ++current;
                bytes.push_back(-1);
            }
            else {
                bytes.push_back(strtoul(current, &current, 16));
            }
        }
        return bytes;
        };

    auto dosHeader = (PIMAGE_DOS_HEADER)module;
    auto ntHeaders = (PIMAGE_NT_HEADERS)((std::uint8_t*)module + dosHeader->e_lfanew);

    auto sizeOfImage = ntHeaders->OptionalHeader.SizeOfImage;
    auto patternBytes = pattern_to_byte(pat);
    auto scanBytes = reinterpret_cast<std::uint8_t*>(module);

    auto s = patternBytes.size();
    auto d = patternBytes.data();

    for (auto i = 0ul; i < sizeOfImage - s; ++i) {
        bool found = true;
        for (auto j = 0ul; j < s; ++j) {
            if (scanBytes[i + j] != d[j] && d[j] != -1) {
                found = false;
                break;
            }
        }
        if (found) {
            return &scanBytes[i];
        }
    }

    return {};
}

memory::address_t memory::find_interface(HMODULE module, const char* inter) {
    memory::address_t fn = GetProcAddress(module, "CreateInterface");
    if (fn.pointer == NULL)
        return nullptr;

    return fn.get<void* (*)(const char*, int)>()(inter, 0);
}

memory::address_t memory::find_virtual(memory::address_t ptr, const int& idx) {
    return (*ptr.get<LPVOID**>())[idx];
}

HMODULE memory::get_module(const char* name) {
    HMODULE out{};
    while (true) {
        out = GetModuleHandleA(name);
        if (out)
            break;
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    return out;
}
