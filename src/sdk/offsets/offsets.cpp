#include "offsets.h"
#include "../classes/interfaces/entity_system.h"
#include "../classes/interfaces/schema_system.h"
#include "../classes/classes.h"

void sdk::c_offsets::init_pattern(HMODULE module, memory::address_t& out, const char* name, const char* pattern) {
    out = memory::find_pattern(module, pattern);
    if (!out.is_valid())
        this->failed_pattern = true;
}

template <typename T = memory::address_t>
T init_interface(HMODULE module, const char* name) {
    memory::address_t out = memory::find_interface(module, name);
    if (out.is_valid()) 
        return out.get<T>();
    else
        return nullptr; 
}

bool sdk::c_offsets::c_modules::initialize() {
    this->client = memory::get_module(("client.dll"));
    this->engine = memory::get_module(("engine2.dll"));
    this->schema_system = memory::get_module(("schemasystem.dll"));
    this->game_overlay_renderer = memory::get_module(("GameOverlayRenderer64.dll"));
    this->input_system = memory::get_module(("inputsystem.dll"));

    if (!this->client || !this->engine || !this->schema_system || !this->game_overlay_renderer || !this->input_system) {
        return false;
    }

    return true;
}

bool sdk::c_offsets::c_interfaces::initialize() {
    this->entity_system = *sdk::g_offsets.patterns.entity_system.resolve<sdk::interfaces::entity_system**>();
    this->schema_system = init_interface<sdk::interfaces::schema_system*>(sdk::g_offsets.modules.schema_system, ("SchemaSystem_001"));
    this->engine = init_interface<sdk::interfaces::engine*>(sdk::g_offsets.modules.engine, ("Source2EngineToClient001"));
    this->input_system = init_interface<void*>(sdk::g_offsets.modules.input_system, ("InputSystemVersion001"));
    this->view_render = *sdk::g_offsets.patterns.view_render.resolve<void**>();

	if (!this->schema_system || !this->engine || !this->input_system) {
        return false;
    }

    return true;
}

bool sdk::c_offsets::initialize() {
    if (!this->modules.initialize() || !this->patterns.initialize() || !this->interfaces.initialize())
        return false;
  
    return true;
}
