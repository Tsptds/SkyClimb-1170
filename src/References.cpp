#pragma once

namespace ModSettings {
    
    float climbDelay = 0.0f;    // Set debounce delay
    bool Enable_Stamina_Consumption = false;
    float Stamina_Damage = 25.0f;
}

// Global variable, hate doing this
RE::COL_LAYER lastHitObject;
// switch for layer hit logs
bool logLayer = false;

float PlayerScale = 1.0f;

namespace GameReferences {
    RE::TESObjectREFR *vaultMarkerRef;
    RE::TESObjectREFR *lowMarkerRef;
    RE::TESObjectREFR *medMarkerRef;
    RE::TESObjectREFR *highMarkerRef;

    RE::TESObjectREFR *vaultActivatorRef;
    RE::TESObjectREFR *lowActivatorRef;
    RE::TESObjectREFR *medActivatorRef;
    RE::TESObjectREFR *highActivatorRef;

    RE::TESObjectREFR *indicatorRef;
}