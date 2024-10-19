#pragma once

#include "REL/Relocation.h"
#include "SKSE/Trampoline.h"

struct PlayerCanActivateFurniture {
    static bool Install();
    static bool thunk(RE::PlayerCharacter* a_player, RE::TESObjectREFR* a_furniture);

    using CanActivateFurniture_t = bool (*)(RE::PlayerCharacter*, RE::TESObjectREFR*);
    inline static REL::Relocation<CanActivateFurniture_t> func;
};
