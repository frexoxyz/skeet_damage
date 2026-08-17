#pragma once
#include <atomic>
#include <cstdint>

namespace sdk::schema {
	int16_t schema_get_offset(const char* className, const char* keyName);
}

#define VALVE_SCHEMA_FIELD(return_type, field_name, field_class, field_name_in_schema) \
    return_type& field_name() const noexcept { \
        static std::atomic<std::int16_t> cached_offset{ 0 }; \
        std::int16_t offset = cached_offset.load(std::memory_order_relaxed); \
        if (offset == 0) { \
            offset = sdk::schema::schema_get_offset((field_class), (field_name_in_schema)); \
            cached_offset.store(offset, std::memory_order_relaxed); \
        } \
        return *reinterpret_cast<return_type*>(reinterpret_cast<std::uintptr_t>(this) + offset); \
    }
#define VALVE_FIELD( return_type, field_name, offset )                                                              \
    __forceinline std::add_lvalue_reference_t< return_type > field_name( ) const {                                  \
        return *reinterpret_cast< return_type* >( reinterpret_cast< std::uintptr_t >( this ) + offset );         \
    }

