#pragma once

void ToggleJumpingInternal(bool enabled) {
    RE::ControlMap::GetSingleton()->ToggleControls(RE::ControlMap::UEFlag::kJumping, enabled);
}

void ToggleJumping(RE::StaticFunctionTag*, bool enabled) { ToggleJumpingInternal(enabled); }

float PlayerVsObjectAngle(const RE::NiPoint3 &objPoint) {
    const auto player = RE::PlayerCharacter::GetSingleton();
    if (!player) {
        return 0.0f;  // Return a safe default if the player singleton is null
    }

    // Get the vector from the player's head to the object
    RE::NiPoint3 playerToObject = objPoint - player->GetPosition();
    playerToObject.z -= 120.0f;  // Adjust for head level

    // Normalize the vector
    const float distance = playerToObject.Length();
    if (distance == 0.0f) {
        return 0.0f;  // Avoid division by zero
    }
    playerToObject /= distance;

    // Get the player's forward direction in the XY plane
    const float playerYaw = player->data.angle.z;
    RE::NiPoint3 playerForwardDir{std::sin(playerYaw), std::cos(playerYaw), 0.0f};

    // Dot product between player's forward direction and the object direction
    const float dot = playerToObject.x * playerForwardDir.x + playerToObject.y * playerForwardDir.y;

    // Clamp the dot product to avoid domain errors in acos
    const float clampedDot = std::clamp(dot, -1.0f, 1.0f);

    // Calculate the angle in degrees
    return std::acos(clampedDot) * 57.2958f;  // radToDeg constant
}

void LastObjectHitType(RE::COL_LAYER obj) { lastHitObject = obj; }

float RayCast(RE::NiPoint3 rayStart, RE::NiPoint3 rayDir, float maxDist, RE::hkVector4 &normalOut,
              RE::COL_LAYER layerMask) {
    const auto player = RE::PlayerCharacter::GetSingleton();
    if (!player) {
        normalOut = RE::hkVector4(0.0f, 0.0f, 0.0f, 0.0f);
        return maxDist;  // Return maxDist if player is null
    }

    const auto bhkWorld = player->GetParentCell()->GetbhkWorld();
    if (!bhkWorld) {
        normalOut = RE::hkVector4(0.0f, 0.0f, 0.0f, 0.0f);
        return maxDist;  // Return maxDist if Havok world is unavailable
    }

    RE::bhkPickData pickData;
    const auto havokWorldScale = RE::bhkWorld::GetWorldScale();

    // Set ray start and end points (scaled to Havok world)
    pickData.rayInput.from = rayStart * havokWorldScale;
    pickData.rayInput.to = (rayStart + rayDir * maxDist) * havokWorldScale;

    // Set the collision filter info to exclude the player
    uint32_t collisionFilterInfo = 0;
    player->GetCollisionFilterInfo(collisionFilterInfo);
    pickData.rayInput.filterInfo = (collisionFilterInfo & 0xFFFF0000) | static_cast<uint32_t>(layerMask);

    // Perform the raycast
    if (bhkWorld->PickObject(pickData) && pickData.rayOutput.HasHit()) {
        normalOut = pickData.rayOutput.normal;

        const uint32_t layerIndex = pickData.rayOutput.rootCollidable->broadPhaseHandle.collisionFilterInfo & 0x7F;

        if (layerIndex == 0) {
            return -1.0f;  // Invalid layer hit
        }

        // Optionally log the layer hit
        // if (logLayer) logger::info("\nLayer hit: {}", layerIndex);

        // Check for useful collision layers
        switch (static_cast<RE::COL_LAYER>(layerIndex)) {
            case RE::COL_LAYER::kStatic:
            case RE::COL_LAYER::kCollisionBox:
            case RE::COL_LAYER::kTerrain:
            case RE::COL_LAYER::kGround:
            case RE::COL_LAYER::kProps:
                // Update last hit object type
                LastObjectHitType(static_cast<RE::COL_LAYER>(layerIndex));
                return maxDist * pickData.rayOutput.hitFraction;

            default:
                return -1.0f;  // Ignore unwanted layers
        }
    }

    // No hit
    normalOut = RE::hkVector4(0.0f, 0.0f, 0.0f, 0.0f);
    // if (logLayer) logger::info("Nothing hit");

    return maxDist;
}

bool IsActorUsingFurniture(RE::Actor* actor, RE::TESObjectREFR* furnitureRef = nullptr) {
    if (!actor) {
        return false;  // Invalid actor
    }

    // Access the actor's process data
    auto process = actor->currentProcess;
    if (!process) {
        return false;  // No active process
    }

    // Check if the actor is interacting with furniture
    auto furnitureInteraction = process->GetOccupiedFurniture();
    if (!furnitureInteraction) {
        return false;  // Actor is not using any furniture
    }

    // If a specific furniture reference is provided, compare it
    // if (furnitureRef && furnitureInteraction != furnitureRef) {
    //    return false;  // Actor is using furniture, but not the specified one
    //}

    return true;  // Actor is using furniture (or the specified furniture)
}

bool IsParkourActive() {
    auto player = RE::PlayerCharacter::GetSingleton();
    auto ui = RE::UI::GetSingleton();
    if (!player || !ui) return false;

    if (IsActorUsingFurniture(player) || player->IsWeaponDrawn()) {
        return false;
    }

    // Check if the game is paused
    if (ui->GameIsPaused()) {
        return false;
    }

    // List of disqualifying menu names
    const std::string_view excludedMenus[] = {RE::BarterMenu::MENU_NAME,       RE::ConsoleNativeUIMenu::MENU_NAME,
                                              RE::ContainerMenu::MENU_NAME,    RE::CraftingMenu::MENU_NAME,
                                              RE::CreationClubMenu::MENU_NAME, RE::DialogueMenu::MENU_NAME,
                                              RE::FavoritesMenu::MENU_NAME,    RE::GiftMenu::MENU_NAME,
                                              RE::InventoryMenu::MENU_NAME,    RE::JournalMenu::MENU_NAME,
                                              RE::LevelUpMenu::MENU_NAME,      RE::LockpickingMenu::MENU_NAME,
                                              RE::MagicMenu::MENU_NAME,        RE::MapMenu::MENU_NAME,
                                              RE::MessageBoxMenu::MENU_NAME,   RE::MistMenu::MENU_NAME,
                                              RE::RaceSexMenu::MENU_NAME,      RE::SleepWaitMenu::MENU_NAME,
                                              RE::StatsMenu::MENU_NAME,        RE::TrainingMenu::MENU_NAME,
                                              RE::TweenMenu::MENU_NAME};

    // Check if any of the excluded menus are open
    for (const std::string_view menuName : excludedMenus) {
        if (ui->IsMenuOpen(menuName)) {
            return false;
        }
    }

    return true;
}

bool PlayerIsGrounded() {
    const auto player = RE::PlayerCharacter::GetSingleton();
    if (!player) {
        return false;  // Early exit if the player is null
    }

    const auto charController = player->GetCharController();
    if (!charController) {
        return false;  // Early exit if the character controller is null
    }

    // Check if the player is in the air (jumping flag)
    if (charController->flags.any(RE::CHARACTER_FLAGS::kJumping)) {
        return false;
    }

    // Raycast parameters
    const auto playerPos = player->GetPosition();
    const RE::NiPoint3 groundedRayStart(playerPos.x, playerPos.y, playerPos.z + 128.0f);
    const RE::NiPoint3 groundedRayDir(0.0f, 0.0f, -1.0f);
    const float groundedCheckDist = 148.0f;  // 128 + 20

    // Perform the raycast
    RE::hkVector4 normalOut(0.0f, 0.0f, 0.0f, 0.0f);
    float groundedRayDist =
        RayCast(groundedRayStart, groundedRayDir, groundedCheckDist, normalOut, RE::COL_LAYER::kLOS);

    // Check if the raycast hit the ground
    return groundedRayDist != groundedCheckDist && groundedRayDist != -1.0f;
}

bool PlayerIsInWater() {
    const auto player = RE::PlayerCharacter::GetSingleton();
    if (!player) {
        return false;  // Early exit if the player is null
    }

    const auto parentCell = player->GetParentCell();
    if (!parentCell) {
        return false;  // Early exit if the parent cell is null
    }

    // Get the player's current position
    const auto playerPos = player->GetPosition();

    // Retrieve the water level at the player's position
    float waterLevel = 0.0f;
    if (!parentCell->GetWaterHeight(playerPos, waterLevel)) {
        return false;  // If water height cannot be determined, player is not in water
    }

    // Check if the player's position is significantly below the water level
    return (playerPos.z - waterLevel) < -50.0f;
}

bool PlayerIsOnStairs() {
    const auto player = RE::PlayerCharacter::GetSingleton();
    if (!player) {
        return false;  // Early exit if the player is null
    }

    const auto charController = player->GetCharController();
    return charController && charController->flags.any(RE::CHARACTER_FLAGS::kOnStairs);
}


float magnitudeXY(float x, float y) { return sqrt(x * x + y * y); }

int LedgeCheck(RE::NiPoint3 &ledgePoint, RE::NiPoint3 checkDir, float minLedgeHeight, float maxLedgeHeight) {
    const auto player = RE::PlayerCharacter::GetSingleton();
    const auto playerPos = player->GetPosition();

    // Constants adjusted for player scale
    float startZOffset = 100 * PlayerScale;
    float playerHeight = 120 * PlayerScale;
    float minUpCheck = 100 * PlayerScale;
    float maxUpCheck = (maxLedgeHeight - startZOffset) + 20 * PlayerScale;
    float fwdCheckStep = 8 * PlayerScale;
    int fwdCheckIterations = 15;
    float minLedgeFlatness = 0.5;
    float ledgeHypotenuse = 0.75;

    RE::hkVector4 normalOut(0, 0, 0, 0);

    // Upward raycast to check for headroom
    RE::NiPoint3 upRayStart = playerPos + RE::NiPoint3(0, 0, startZOffset);
    RE::NiPoint3 upRayDir(0, 0, 1);

    float upRayDist = RayCast(upRayStart, upRayDir, maxUpCheck, normalOut, RE::COL_LAYER::kLOS);
    if (upRayDist < minUpCheck) {
        return -1;
    }

    // Forward raycast initialization
    RE::NiPoint3 fwdRayStart = upRayStart + upRayDir * (upRayDist - 10);
    RE::NiPoint3 downRayDir(0, 0, -1);

    bool foundLedge = false;
    float normalZ = 0;

    // Incremental forward raycast to find a ledge
    for (int i = 0; i < fwdCheckIterations; i++) {
        float fwdRayDist = RayCast(fwdRayStart, checkDir, fwdCheckStep * i, normalOut, RE::COL_LAYER::kLOS);
        if (fwdRayDist < fwdCheckStep * i) {
            continue;
        }

        // Downward raycast to detect ledge point
        RE::NiPoint3 downRayStart = fwdRayStart + checkDir * fwdRayDist;
        float downRayDist =
            RayCast(downRayStart, downRayDir, startZOffset + maxUpCheck, normalOut, RE::COL_LAYER::kLOS);

        ledgePoint = downRayStart + downRayDir * downRayDist;
        normalZ = normalOut.quad.m128_f32[2];

        // Validate ledge based on height and flatness
        if (ledgePoint.z < playerPos.z + minLedgeHeight || ledgePoint.z > playerPos.z + maxLedgeHeight ||
            downRayDist < 10 || normalZ < minLedgeFlatness) {
            continue;
        }

        foundLedge = true;
        break;
    }

    if (!foundLedge) {
        return -1;
    }

    // Ensure there is sufficient headroom for the player to stand
    float headroomBuffer = 10 * PlayerScale;
    RE::NiPoint3 headroomRayStart = ledgePoint + upRayDir * headroomBuffer;
    float headroomRayDist =
        RayCast(headroomRayStart, upRayDir, playerHeight - headroomBuffer, normalOut, RE::COL_LAYER::kLOS);

    if (headroomRayDist < playerHeight - headroomBuffer) {
        return -1;
    }

    float ledgePlayerDiff = ledgePoint.z - playerPos.z;

    if (ledgePlayerDiff > 175 * PlayerScale) {
        return 2;  // High ledge
    } else if (ledgePlayerDiff >= 120 * PlayerScale) {
        return 1;  // Medium ledge
    } else {
        // Additional horizontal and vertical checks for low ledge
        double horizontalDistance = sqrt(pow(ledgePoint.x - playerPos.x, 2) + pow(ledgePoint.y - playerPos.y, 2));
        double verticalDistance = abs(ledgePlayerDiff);

        if (!PlayerIsOnStairs() && horizontalDistance < verticalDistance * ledgeHypotenuse) {
            return 5;  // Low ledge
        }
    }

    return -1;
}

int VaultCheck(RE::NiPoint3 &ledgePoint, RE::NiPoint3 checkDir, float vaultLength, float maxElevationIncrease,
               float minVaultHeight, float maxVaultHeight) {
    const auto player = RE::PlayerCharacter::GetSingleton();
    const auto playerPos = player->GetPosition();

    RE::hkVector4 normalOut(0, 0, 0, 0);

    float headHeight = 120 * PlayerScale;

    // Forward raycast to check for a vaultable surface
    RE::NiPoint3 fwdRayStart = playerPos + RE::NiPoint3(0, 0, headHeight);
    float fwdRayDist = RayCast(fwdRayStart, checkDir, vaultLength, normalOut, RE::COL_LAYER::kLOS);

    if (lastHitObject == RE::COL_LAYER::kTerrain || fwdRayDist < vaultLength) {
        return -1;  // Not vaultable if terrain or insufficient distance
    }

    // Backward ray to check for obstructions behind the vaultable surface
    RE::NiPoint3 backwardRayStart = fwdRayStart + checkDir * (fwdRayDist - 2) + RE::NiPoint3(0, 0, 5);
    float backwardRayDist = RayCast(backwardRayStart, checkDir, 50.0f, normalOut, RE::COL_LAYER::kLOS);

    if (backwardRayDist > 0 && backwardRayDist < 50.0f) {
        return -1;  // Obstruction behind the vaultable surface
    }

    // Downward raycast initialization
    int downIterations = static_cast<int>(std::floor(vaultLength / 5.0f));
    RE::NiPoint3 downRayDir(0, 0, -1);

    bool foundVaulter = false;
    float foundVaultHeight = -10000.0f;
    bool foundLanding = false;
    float foundLandingHeight = 10000.0f;

    // Incremental downward raycasts
    for (int i = 0; i < downIterations; i++) {
        float iDist = static_cast<float>(i) * 5.0f;
        RE::NiPoint3 downRayStart = playerPos + checkDir * iDist;
        downRayStart.z = fwdRayStart.z;

        float downRayDist = RayCast(downRayStart, downRayDir, headHeight + 100.0f, normalOut, RE::COL_LAYER::kLOS);
        float hitHeight = (fwdRayStart.z - downRayDist) - playerPos.z;

        // Check hit height for vaultable surfaces
        if (hitHeight > maxVaultHeight) {
            return -1;  // Too high to vault
        } else if (hitHeight > minVaultHeight && hitHeight < maxVaultHeight) {
            if (hitHeight >= foundVaultHeight) {
                foundVaultHeight = hitHeight;
                foundLanding = false;
            }
            ledgePoint = downRayStart + downRayDir * downRayDist;
            foundVaulter = true;
        } else if (foundVaulter && hitHeight < minVaultHeight) {
            foundLandingHeight = std::min(hitHeight, foundLandingHeight);
            foundLanding = true;
        }
    }

    // Final validation for vault
    if (foundVaulter && foundLanding && foundLandingHeight < maxElevationIncrease) {
        ledgePoint.z = playerPos.z + foundVaultHeight;
        if (!PlayerIsOnStairs()) {
            return 3;  // Vault successful
        }
    }

    return -1;  // Vault failed
}