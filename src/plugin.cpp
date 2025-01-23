#include "ButtonListener.cpp"
#include "Util.cpp"
namespace logger = SKSE::log;

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
    RE::TESObjectREFR *indicatorRef;
}

void SetupLog() {
    auto logsFolder = SKSE::log::log_directory();
    if (!logsFolder) SKSE::stl::report_and_fail("SKSE log_directory not provided, logs disabled.");

    auto pluginName = "SkyParkour";
    auto logFilePath = *logsFolder / std::format("{}.log", pluginName);
    auto fileLoggerPtr = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFilePath.string(), true);
    auto loggerPtr = std::make_shared<spdlog::logger>("log", std::move(fileLoggerPtr));

    spdlog::set_default_logger(std::move(loggerPtr));
    spdlog::set_level(spdlog::level::info);
    spdlog::flush_on(spdlog::level::info);

    spdlog::set_pattern("%v");
    //spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%l] [%t] [%s:%#] %v");
}

void ToggleJumpingInternal(bool enabled) {
    RE::ControlMap::GetSingleton()->ToggleControls(RE::ControlMap::UEFlag::kJumping, enabled);
}

void ToggleJumping(RE::StaticFunctionTag *, bool enabled) { ToggleJumpingInternal(enabled); }

//void EndAnimationEarly(RE::StaticFunctionTag *, RE::TESObjectREFR *objectRef) {
//   
//        if(!objectRef) logger::error("Null Player Ref Animation not ended early");
//        else
//            objectRef->NotifyAnimationGraph("IdleFurnitureExit");
//    
//}

void RegisterClimbButton(RE::StaticFunctionTag *, int32_t dxcode) {
    
    const auto mappedButton = ButtonStates::MapToCKIfPossible(dxcode);

    ButtonStates::DXCODE = mappedButton;
    logger::info("Climb key registered: {}", mappedButton);

    
}
void RegisterClimbDelay(RE::StaticFunctionTag*, float delay) {
    ButtonStates::debounceDelay = delay;
    logger::info("Delay Registered {}", ButtonStates::debounceDelay);
}

void RegisterReferences(RE::StaticFunctionTag *, RE::TESObjectREFR *vaultMarkerRef, RE::TESObjectREFR *lowMarkerRef, 
    RE::TESObjectREFR *medMarkerRef, RE::TESObjectREFR* highMarkerRef, RE::TESObjectREFR* indicatorRef) {

    GameReferences::vaultMarkerRef = vaultMarkerRef;
    GameReferences::lowMarkerRef = lowMarkerRef;
    GameReferences::medMarkerRef = medMarkerRef;
    GameReferences::highMarkerRef = highMarkerRef;
    GameReferences::indicatorRef = indicatorRef;
}

bool IsClimbKeyDown(RE::StaticFunctionTag *) {
    return ButtonStates::isDown;
}

bool IsParkourActive(RE::StaticFunctionTag *) {
    auto player = RE::PlayerCharacter::GetSingleton();
    auto ui = RE::UI::GetSingleton();
    if (!player || !ui) return false;

    // Check player's state
    if (player->GetSitSleepState() != RE::SIT_SLEEP_STATE::kNormal || player->IsWeaponDrawn()) {
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

float magnitudeXY(float x, float y) {

    return sqrt(x * x + y * y);

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
        float horizontalDistance = sqrt(pow(ledgePoint.x - playerPos.x, 2) + pow(ledgePoint.y - playerPos.y, 2));
        float verticalDistance = abs(ledgePlayerDiff);

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
    if (!PlayerIsGrounded() || PlayerIsInWater()) {
        ToggleJumpingInternal(true);  // Ensure jump key is re-enabled to prevent being stuck
        return -1;
    }

    // Handle jump key toggling based on ledge point type
    if (useJumpKey) {
        ToggleJumpingInternal(ledgePointType < 0);
    }

    return ledgePointType;
}



bool PapyrusFunctions(RE::BSScript::IVirtualMachine * vm) {

    vm->RegisterFunction("ToggleJumping", "SkyParkourPapyrus", ToggleJumping);

    //vm->RegisterFunction("EndAnimationEarly", "SkyParkourPapyrus", EndAnimationEarly);

    vm->RegisterFunction("UpdateParkourPoint", "SkyParkourPapyrus", UpdateParkourPoint);

    vm->RegisterFunction("IsClimbKeyDown", "SkyParkourPapyrus", IsClimbKeyDown);

    vm->RegisterFunction("RegisterClimbButton", "SkyParkourPapyrus", RegisterClimbButton);

    vm->RegisterFunction("RegisterClimbDelay", "SkyParkourPapyrus", RegisterClimbDelay);

    vm->RegisterFunction("RegisterReferences", "SkyParkourPapyrus", RegisterReferences);

    vm->RegisterFunction("IsParkourActive", "SkyParkourPapyrus", IsParkourActive);

    return true; 
}

extern "C" DLLEXPORT constinit auto SKSEPlugin_Version = []() {
    SKSE::PluginVersionData v;
    v.PluginVersion({Version::MAJOR, Version::MINOR, Version::PATCH});
    v.PluginName("SkyParkour");
    v.AuthorName("Sokco & Tsptds");
    v.UsesAddressLibrary();
    v.UsesUpdatedStructs();
    v.CompatibleVersions({SKSE::RUNTIME_1_6_1130, 
        {(unsigned short)1U, (unsigned short)6U, (unsigned short)1170U, (unsigned short)0U}});

    return v;
}();


void MessageEvent(SKSE::MessagingInterface::Message *message) {
    // logger::info("Message event");
    if (message->type == SKSE::MessagingInterface::kDataLoaded) {
        ButtonEventListener::Register();
    }
}
extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface *a_skse) {
    SetupLog();

    SKSE::Init(a_skse);
    logger::info("SkyParkour is Starting!");
    SKSE::GetPapyrusInterface()->Register(PapyrusFunctions); 
    SKSE::GetMessagingInterface()->RegisterListener(MessageEvent);
    
    return true;
}
