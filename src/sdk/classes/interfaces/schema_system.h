#pragma once
#include "../../tools/memory/memory.h"

namespace sdk::interfaces {
    struct schema_class_field {
        const char* m_name;
        char pad0[0x8];
        short m_offset;
        char pad1[0xE];
    };

    class schema_class_info {
    public:
        const char* get_name() {
            return *reinterpret_cast<const char**>((unsigned __int64)(this) + 0x8);
        }
        short get_fields_size() {
            return *reinterpret_cast<short*>((unsigned __int64)(this) + 0x24);

        }
        schema_class_field* get_fields() {
            return *reinterpret_cast<schema_class_field**>((unsigned __int64)(this) + 0x30);
        }
    };

    class schema_type_scope {
    public:
        schema_class_info* find_declared_class(const char* className) {
            schema_class_info* rv = nullptr;
            VFUNC(this, void, 2, &rv, className);
            return rv;
        }
    };

    class schema_system {
    public:
        schema_type_scope* find_type_scope_for_module(const char* moduleName) {
            return VFUNC(this, schema_type_scope*, 13, moduleName, nullptr);
       }
    };
}
