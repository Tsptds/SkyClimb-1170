#include "References.cpp"
#include "ButtonListener.cpp"

void DamagePlayerStamina(float amount) {
    const auto player = RE::PlayerCharacter::GetSingleton();
    if (player) {
        player->RestoreActorValue(RE::ACTOR_VALUE_MODIFIERS::kDamage, RE::ActorValue::kStamina, -amount);
    }
}

bool ActivateClimb(int ledgePointType) {
    using namespace GameReferences;
    const auto player = RE::PlayerCharacter::GetSingleton();

    if (ModSettings::Enable_Stamina_Consumption) {
        DamagePlayerStamina(ModSettings::Stamina_Damage);
    }

    if (ledgePointType == 5)
        return lowActivatorRef->ActivateRef(player, 0, nullptr, 0, false);
    else if (ledgePointType == 1)
        return medActivatorRef->ActivateRef(player, 0, nullptr, 0, false);
    else if (ledgePointType == 2)
        return highActivatorRef->ActivateRef(player, 0, nullptr, 0, false);
    else if (ledgePointType == 3)
        return vaultActivatorRef->ActivateRef(player, 0, nullptr, 0, false);
}


int GetLedgePoint(RE::TESObjectREFR *vaultMarkerRef, RE::TESObjectREFR *lowMarkerRef, RE::TESObjectREFR *medMarkerRef,
                  RE::TESObjectREFR *highMarkerRef, RE::TESObjectREFR *indicatorRef, bool enableVaulting,
                  bool enableLedges, float backwardOffset = 55.0f) {
    // Nullptr check for all references
    if (!indicatorRef || !vaultMarkerRef || !medMarkerRef || !highMarkerRef || !lowMarkerRef) {
        return -1;
    }

    const auto player = RE::PlayerCharacter::GetSingleton();
    const auto playerPos = player->GetPosition();

    // Calculate player forward direction (normalized)
    const float playerYaw = player->data.angle.z;  // Player's yaw
    RE::NiPoint3 playerDirFlat{std::sin(playerYaw), std::cos(playerYaw), 0};
    const float dirMagnitude = std::hypot(playerDirFlat.x, playerDirFlat.y);
    playerDirFlat.x /= dirMagnitude;
    playerDirFlat.y /= dirMagnitude;

    // Perform ledge or vault checks
    int selectedLedgeType = -1;
    RE::NiPoint3 ledgePoint;

    if (enableVaulting) {
        selectedLedgeType =
            VaultCheck(ledgePoint, playerDirFlat, 100, 70 * PlayerScale, 40.5f * PlayerScale, 90 * PlayerScale);
    }
    if (selectedLedgeType == -1 && enableLedges) {
        selectedLedgeType = LedgeCheck(ledgePoint, playerDirFlat, 40 * PlayerScale, 250 * PlayerScale);
    }
    if (selectedLedgeType == -1 || PlayerVsObjectAngle(ledgePoint) > 80) {
        return -1;
    }

    // Move indicator to the correct position
    if (indicatorRef->GetParentCell() != player->GetParentCell()) {
        indicatorRef->MoveTo(player->AsReference());
    }

    RE::NiPoint3 backwardAdjustment = playerDirFlat * backwardOffset * PlayerScale;
    indicatorRef->data.location = ledgePoint + RE::NiPoint3(0, 0, 5);  // Offset upwards slightly
    indicatorRef->Update3DPosition(true);
    indicatorRef->data.angle = RE::NiPoint3(0, 0, atan2(playerDirFlat.x, playerDirFlat.y));

    // Select appropriate ledge marker and adjustments
    RE::TESObjectREFR *ledgeMarker = nullptr;
    float zAdjust = 0.0f;
    switch (selectedLedgeType) {
        case 5:  // Low ledge
            ledgeMarker = lowMarkerRef;
            zAdjust = -80 * PlayerScale;
            backwardAdjustment = playerDirFlat * 50 * PlayerScale;  // Adjust backward offset
            break;
        case 1:  // Medium ledge
            ledgeMarker = medMarkerRef;
            zAdjust = -155 * PlayerScale;
            break;
        case 2:  // High ledge
            ledgeMarker = highMarkerRef;
            zAdjust = -200 * PlayerScale;
            break;
        default:  // Vault
            ledgeMarker = vaultMarkerRef;
            zAdjust = -60 * PlayerScale;
            break;
    }

    // Ensure ledge marker is valid
    if (!ledgeMarker) {
        return -1;
    }

    if (ledgeMarker->GetParentCell() != player->GetParentCell()) {
        ledgeMarker->MoveTo(player->AsReference());
    }

    // Position ledge marker with adjustments
    ledgeMarker->SetPosition(
        {ledgePoint.x - backwardAdjustment.x, ledgePoint.y - backwardAdjustment.y, ledgePoint.z + zAdjust});
    ledgeMarker->data.angle = RE::NiPoint3(0, 0, atan2(playerDirFlat.x, playerDirFlat.y));

    return selectedLedgeType;
}

int UpdateParkourPoint(RE::StaticFunctionTag *, bool useJumpKey, bool enableVaulting, bool enableLedges) {
    PlayerScale = GetScale();

    using namespace GameReferences;
    const int ledgePointType = GetLedgePoint(vaultMarkerRef, lowMarkerRef, medMarkerRef, highMarkerRef, indicatorRef,
                                             enableVaulting, enableLedges);

    // If player is not grounded or is in water, reset jump key and return early
    if (!IsParkourActive() || !PlayerIsGrounded() || PlayerIsInWater() || ledgePointType == -1) {
        ToggleJumpingInternal(true);  // Ensure jump key is re-enabled to prevent being stuck
        indicatorRef->Disable();
        return -1;
    }

    indicatorRef->Enable(false);  // Don't reset inventory

    // Handle jump key toggling based on ledge point type
    if (useJumpKey) {
        ToggleJumpingInternal(ledgePointType == -1);
    }
    if (ButtonStates::isDown) {
        bool success = ActivateClimb(ledgePointType);

        /*if (ModSettings::Enable_Stamina_Consumption && success) {
            DamagePlayerStamina(ModSettings::Stamina_Damage);
        }*/
        logger::info("activated: {}", success);
    }
    return ledgePointType;
}