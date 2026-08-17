#include "schema.h"
#include "../schema_system.h"
#include "../../classes.h"
#include "../../../offsets/offsets.h"

int16_t sdk::schema::schema_get_offset(const char* class_name, const char* key_name) {
    if (!sdk::g_offsets.interfaces.schema_system)
        return 0;

    const uint64_t key_name_key = fnv1a::hash_64(key_name);

    sdk::interfaces::schema_type_scope* type_scope = sdk::g_offsets.interfaces.schema_system->find_type_scope_for_module("client.dll");
    if (!type_scope)
        return 0;

    sdk::interfaces::schema_class_info* class_info = type_scope->find_declared_class(class_name);
    if (!class_info)
        return 0;

    const short fields_size = class_info->get_fields_size();
    sdk::interfaces::schema_class_field* fields = class_info->get_fields();
    if (!fields)
        return 0;

    for (int i = 0; i < fields_size; ++i) {
        if (fnv1a::hash_64(fields[i].m_name) == key_name_key)
            return fields[i].m_offset;
    }

    return 0;
}
