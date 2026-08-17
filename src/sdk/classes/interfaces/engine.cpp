#include "engine.h"
#include "../../tools/memory/memory.h"

bool sdk::interfaces::engine::in_game() {
    if (!this)
        return false;

    return VFUNC(this, bool, 38);
}

bool sdk::interfaces::engine::is_connected() {
    if (!this)
        return false;

    return VFUNC(this, bool, 39);
}