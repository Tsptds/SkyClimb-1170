#include "ButtonListener.cpp"
#include "Util.cpp"
namespace logger = SKSE::log;

// Global variable, hate doing this
RE::COL_LAYER lastHitObject;
// switch for layer hit logs
bool logLayer = false;

float PlayerScale = 1.0f;

void SetupLog() {
    auto logsFolder = SKSE::log::log_directory();
    if (!logsFolder) SKSE::stl::report_and_fail("SKSE log_directory not provided, logs disabled.");

    auto pluginName = "SkyClimb";
    auto logFilePath = *logsFolder / std::format("{}.log", pluginName);
    auto fileLoggerPtr = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFilePath.string(), true);
    auto loggerPtr = std::make_shared<spdlog::logger>("log", std::move(fileLoggerPtr));

    spdlog::set_default_logger(std::move(loggerPtr));
    spdlog::set_level(spdlog::level::info);
    spdlog::flush_on(spdlog::level::info);

    spdlog::set_pattern("%v");
    //spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%l] [%t] [%s:%#] %v");
}

/*
void SetupLog() {
    auto logsFolder = SKSE::log::log_directory();
    if (!logsFolder) SKSE::stl::report_and_fail("SKSE log_directory not provided, logs disabled.");
    auto pluginName = SKSE::PluginDeclaration::GetSingleton()->GetName();
    auto logFilePath = *logsFolder / std::format("{}.log", pluginName);
    auto fileLoggerPtr = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFilePath.string(), true);
    auto loggerPtr = std::make_shared<spdlog::logger>("log", std::move(fileLoggerPtr));
    spdlog::set_default_logger(std::move(loggerPtr));
    spdlog::set_level(spdlog::level::trace);
    spdlog::flush_on(spdlog::level::trace);
}
*/

/*
struct OurEventSink : public RE::BSTEventSink<SKSE::CameraEvent> {
    RE::BSEventNotifyControl ProcessEvent(const SKSE::CameraEvent* event, RE::BSTEventSource<SKSE::CameraEvent> *) {
        
        
        RE::TESCameraState *furnitureState = RE::PlayerCamera::GetSingleton()->cameraStates[5].get();

        if (event->newState->id == furnitureState->id) {
            
             RE::PlayerCamera::GetSingleton()->SetState(event->oldState);
             logger::info("Camera state reverted");
        }
        
        
        return RE::BSEventNotifyControl::kContinue;



    }
};
*/

std::string SayHello(RE::StaticFunctionTag *) { 

    return "Hello from SkyClimb 6!"; 
}

float getSign(float x) {
    if (x < 0) return -1;
    else return 1;
}

void ToggleJumpingInternal(bool enabled) {
    RE::ControlMap::GetSingleton()->ToggleControls(RE::ControlMap::UEFlag::kJumping, enabled);
}

void ToggleJumping(RE::StaticFunctionTag *, bool enabled) { ToggleJumpingInternal(enabled); }

void EndAnimationEarly(RE::StaticFunctionTag *, RE::TESObjectREFR *objectRef) {
   
        if(!objectRef) logger::error("Null Player Ref Animation not ended early");
        else
            objectRef->NotifyAnimationGraph("IdleFurnitureExit");
    
}

void RegisterClimbButton(RE::StaticFunctionTag *, int32_t dxcode) {
    
    const auto mappedButton = ButtonStates::MapToCKIfPossible(dxcode);

    ButtonStates::DXCODE = mappedButton;
    logger::info("Climb key registered: {}", mappedButton);

    
}
void RegisterClimbDelay(RE::StaticFunctionTag*, float delay) {
    ButtonStates::debounceDelay = delay;
    logger::info("Delay Registered {}", ButtonStates::debounceDelay);
}

bool IsClimbKeyDown(RE::StaticFunctionTag *) {
    return ButtonStates::isDown;
}
bool IsParkourActive(RE::StaticFunctionTag *) {
    auto player = RE::PlayerCharacter::GetSingleton();
    if (!player) return false;
    
    //return playerRef.GetSleepState() == 0 && playerRef.GetSitState() == 0 && Utility.IsInMenuMode() == false && playerRef.IsWeaponDrawn() == false
    return player->GetSitSleepState() == RE::SIT_SLEEP_STATE::kNormal &&
           RE::UI::GetSingleton()->GameIsPaused() == false && player->IsWeaponDrawn() == false;
}


float PlayerVsObjectAngle(RE::NiPoint3 objPoint) {
    const auto player = RE::PlayerCharacter::GetSingleton();

    // Get the vector from the player to the object
    RE::NiPoint3 playerToObject =
        objPoint - (player->GetPosition() + RE::NiPoint3(0, 0, 120));  // Adding some height to match head level

    // Normalize the vector
    playerToObject /= playerToObject.Length();

    // Get the player's forward direction as a unit vector (based on player's yaw)
    float playerYaw = player->data.angle.z;  // Get the player's yaw angle
    RE::NiPoint3 playerForwardDir;
    playerForwardDir.x = std::sin(playerYaw);
    playerForwardDir.y = std::cos(playerYaw);
    playerForwardDir.z = 0;  // We only care about the direction in the XY plane

    // Normalize the player's forward direction
    playerForwardDir /= playerForwardDir.Length();

    // Calculate the dot product between the player's forward direction and the object direction
    float dot = playerToObject.Dot(playerForwardDir);

    // Convert radians to degrees
    const float radToDeg = 57.2958f;

    // Calculate the angle between the two vectors
    return acos(dot) * radToDeg;
}


void LastObjectHitType(RE::COL_LAYER obj) { lastHitObject = obj; }


float RayCast(RE::NiPoint3 rayStart, RE::NiPoint3 rayDir, float maxDist, RE::hkVector4 &normalOut,/* bool logLayer,*/ RE::COL_LAYER layerMask) {

    RE::NiPoint3 rayEnd = rayStart + rayDir * maxDist;

    const auto bhkWorld = RE::PlayerCharacter::GetSingleton()->GetParentCell()->GetbhkWorld();
    if (!bhkWorld) {
        return maxDist;
    }

    RE::bhkPickData pickData;

    const auto havokWorldScale = RE::bhkWorld::GetWorldScale();

    pickData.rayInput.from = rayStart * havokWorldScale;
    pickData.rayInput.to = rayEnd * havokWorldScale;
    //pickData.rayInput.enableShapeCollectionFilter = false;
    //pickData.rayInput.filterInfo = RE::bhkCollisionFilter::GetSingleton()->GetNewSystemGroup() << 16 | SKSE::stl::to_underlying(layerMask);
    
    // Pass player collision to filter it out, ONLY PLAYER
    const auto player = RE::PlayerCharacter::GetSingleton();
    uint32_t collisionFilterInfo = 0;
    player->GetCollisionFilterInfo(collisionFilterInfo);
    pickData.rayInput.filterInfo = (static_cast<uint32_t>(collisionFilterInfo >> 16) << 16) | static_cast<uint32_t>(layerMask);

    if (bhkWorld->PickObject(pickData); pickData.rayOutput.HasHit()) {

        normalOut = pickData.rayOutput.normal;

        uint32_t layerIndex = pickData.rayOutput.rootCollidable->broadPhaseHandle.collisionFilterInfo & 0x7F;

        if (!layerIndex) {
            return -1;
        }

        if (logLayer) logger::info("\nlayer hit: {}", layerIndex);

        //fail if hit a character
        switch (static_cast<RE::COL_LAYER>(layerIndex)) {
            case RE::COL_LAYER::kStatic:
            case RE::COL_LAYER::kCollisionBox:
            case RE::COL_LAYER::kTerrain:
            case RE::COL_LAYER::kGround:
            case RE::COL_LAYER::kProps:
            //case RE::COL_LAYER::kClutter:
  
                // update last hit collision object to check later
                LastObjectHitType(static_cast<RE::COL_LAYER>(layerIndex));
                // hit something useful!
                return maxDist * pickData.rayOutput.hitFraction;

            default: {
                return -1;

            } break;
        }

    }

    if (logLayer) logger::info("nothing hit");

    normalOut = RE::hkVector4(0, 0, 0, 0);

    //didn't hit anything!
    return maxDist;
}

float magnitudeXY(float x, float y) {

    return sqrt(x * x + y * y);

}



bool PlayerIsGrounded() {

    const auto player = RE::PlayerCharacter::GetSingleton();
    const auto playerPos = player->GetPosition();

    RE::hkVector4 normalOut(0, 0, 0, 0);

    // grounded check
    float groundedCheckDist = 128 + 20;

    if (const auto charController = player->GetCharController()) {
        
        if (charController->flags.any(RE::CHARACTER_FLAGS::kJumping)) {
            return false;
        }

        RE::NiPoint3 groundedRayStart;
        groundedRayStart.x = playerPos.x;
        groundedRayStart.y = playerPos.y;
        groundedRayStart.z = playerPos.z + 128;

        RE::NiPoint3 groundedRayDir(0, 0, -1);

        float groundedRayDist =
            RayCast(groundedRayStart, groundedRayDir, groundedCheckDist, normalOut, /*false,*/ RE::COL_LAYER::kLOS);

        if (groundedRayDist == groundedCheckDist || groundedRayDist == -1) {
            return false;
        }
    }
    return true;
}

bool PlayerIsInWater() {
    
    const auto player = RE::PlayerCharacter::GetSingleton();
    const auto playerPos = player->GetPosition();

    // temp water swimming fix
    float waterLevel;
    player->GetParentCell()->GetWaterHeight(playerPos, waterLevel);

    if (playerPos.z - waterLevel < -50) {
        return true;
    }

    return false;
}

bool PlayerIsOnStairs(){
    const auto player = RE::PlayerCharacter::GetSingleton();
    if (player) {
        if (const auto charController = player->GetCharController()) {
            return charController->flags.any(RE::CHARACTER_FLAGS::kOnStairs);
        }
    }

    return false;
}


int LedgeCheck(RE::NiPoint3 &ledgePoint, RE::NiPoint3 checkDir, float minLedgeHeight, float maxLedgeHeight) {

    const auto player = RE::PlayerCharacter::GetSingleton();
    const auto playerPos = player->GetPosition();

    float startZOffset = 100 * PlayerScale;  // how high to start the raycast above the feet of the player
    float playerHeight = 120 * PlayerScale;  // how much headroom is needed
    float minUpCheck = 100 * PlayerScale;    // how low can the roof be relative to the raycast starting point?
    float maxUpCheck = (maxLedgeHeight - startZOffset) + 20 * PlayerScale;  // how high do we even check for a roof?
    float fwdCheck = 8 * PlayerScale; // how much each incremental forward check steps forward               // Default 8
    int fwdCheckIterations = 15;  // how many incremental forward checks do we make?            // Default 15
    float minLedgeFlatness = 0.5;  // 1 is perfectly flat, 0 is completely perpendicular        // Default 0.5
    float ledgeHypotenus = 0.75; // This prevents lowest level grab from triggering on inclined surfaces, larger means less strict. Above 1 has no meaning, never set 0     // Default 0.75

    RE::hkVector4 normalOut(0, 0, 0, 0);

    // raycast upwards to sky, making sure no roof is too low above us
    RE::NiPoint3 upRayStart;
    upRayStart.x = playerPos.x;
    upRayStart.y = playerPos.y;
    upRayStart.z = playerPos.z + startZOffset;

    RE::NiPoint3 upRayDir(0, 0, 1);

    float upRayDist = RayCast(upRayStart, upRayDir, maxUpCheck, normalOut,/* false,*/ RE::COL_LAYER::kLOS);

    if (upRayDist < minUpCheck) {
        logger::info("upRayDist {} < minUpCheck {}", upRayDist, minUpCheck);
        return -1;
    }

    RE::NiPoint3 fwdRayStart = upRayStart + upRayDir * (upRayDist - 10);

    RE::NiPoint3 downRayStart;
    RE::NiPoint3 downRayDir(0, 0, -1);

    bool foundLedge = false;

    float normalZ=0;

    // if nothing above, raycast forwards then down to find ledge
    // incrementally step forward to find closest ledge in front
    for (int i = 0; i < fwdCheckIterations; i++) {
        // raycast forward

        float fwdRayDist = RayCast(fwdRayStart, checkDir, fwdCheck * (float)i, normalOut, /*false,*/ RE::COL_LAYER::kLOS);

        if (fwdRayDist < fwdCheck * (float)i) {
            //logger::info("fwdRayDist {} < fwdCheck {}", fwdRayDist, fwdCheck * (float)i);
            continue;
        }

        // if nothing forward, raycast back down

        downRayStart = fwdRayStart + checkDir * fwdRayDist;

        float downRayDist =
            RayCast(downRayStart, downRayDir, startZOffset + maxUpCheck, normalOut, /*false,*/ RE::COL_LAYER::kLOS);

        ledgePoint = downRayStart + downRayDir * downRayDist;

        /*float*/ normalZ = normalOut.quad.m128_f32[2];

        // if found ledgePoint is too low/high, or the normal is too steep, or the ray hit oddly soon, skip and
        // increment forward again
        if (ledgePoint.z < playerPos.z + minLedgeHeight || ledgePoint.z > playerPos.z + maxLedgeHeight ||
            downRayDist < 10 || normalZ < minLedgeFlatness) {
            //logger::info("{} {} {} {}", ledgePoint.z < playerPos.z + minLedgeHeight, ledgePoint.z > playerPos.z + maxLedgeHeight, downRayDist < 10, normalZ < minLedgeFlatness);
            continue;
        } else {
            foundLedge = true;
            break;
        }
    }

    // if no ledge found, return false
    if (foundLedge == false) {
        return -1;
    }

    // make sure player can stand on top
    float headroomBuffer = 10 * PlayerScale; // default 10

    RE::NiPoint3 headroomRayStart = ledgePoint + upRayDir * headroomBuffer;

    float headroomRayDist = RayCast(headroomRayStart, upRayDir, playerHeight - headroomBuffer, normalOut, /*false,*/ RE::COL_LAYER::kLOS);

    if (headroomRayDist < playerHeight - headroomBuffer) {
        return -1;
    }
    float ledgePlayerDiff = ledgePoint.z - playerPos.z;

    if (logLayer) logger::info("**************\nLedge - player {}", ledgePlayerDiff);
    if (logLayer) logger::info("Flatness {}", normalZ);

    if (ledgePlayerDiff > 175 * PlayerScale) {
        //logger::info("Returned High Ledge");
        //logger::info("Climbing High=> V:{}", ledgePlayerDiff);
        return 2;
    } else if (ledgePlayerDiff >= 120 * PlayerScale) {
        //logger::info("Returned Med Ledge");
        //logger::info("Climbing Med=> V:{}", ledgePlayerDiff);
        return 1;
        
    } else {
        
        // Calculate horizontal and vertical distances
        double horizontalDistance = sqrt(pow(ledgePoint.x - playerPos.x, 2) + pow(ledgePoint.y - playerPos.y, 2));
        double verticalDistance = abs(ledgePoint.z - playerPos.z);
        
        // Check if horizontal distance is more than  vertical distance
        if (!PlayerIsOnStairs()) { 
            if (horizontalDistance < verticalDistance * ledgeHypotenus) {
                //logger::info("Climbing Low=> H:{} V:{}", horizontalDistance, verticalDistance);
                //logger::info("Returned low ledge");
                return 5;
            }
            
        }

    }
    return -1;
   
}

int VaultCheck(RE::NiPoint3 &ledgePoint, RE::NiPoint3 checkDir, float vaultLength, float maxElevationIncrease, float minVaultHeight, float maxVaultHeight) {

    const auto player = RE::PlayerCharacter::GetSingleton();
    const auto playerPos = player->GetPosition();

    RE::hkVector4 normalOut(0, 0, 0, 0);

    float headHeight = 120 * PlayerScale;

    RE::NiPoint3 fwdRayStart;
    fwdRayStart.x = playerPos.x;
    fwdRayStart.y = playerPos.y;
    fwdRayStart.z = playerPos.z + headHeight;

    float fwdRayDist = RayCast(fwdRayStart, checkDir, vaultLength, normalOut, RE::COL_LAYER::kLOS);

    if (fwdRayDist < vaultLength || lastHitObject == RE::COL_LAYER::kTerrain) {
        return -1;
    }

    // Backward ray to check if an object is blocking behind the vaultable surface
    RE::NiPoint3 backwardRayStart = fwdRayStart + checkDir * (fwdRayDist - 2) + RE::NiPoint3(0,0,5);  // Start the ray from the hit point with a z offset, and a starting offset

    // Check for any object within a small distance behind the vaultable object
    float backwardRayDist = RayCast(backwardRayStart, checkDir, 50.0f, normalOut, RE::COL_LAYER::kLOS);
    if (backwardRayDist < 50.0f && backwardRayDist > 0) {
        // An object was found behind the vaultable surface, so cancel the vault
        return -1;
    }

    int downIterations = (int)std::floor(vaultLength / 5.0f) /*24*/;       //Default 5.0f

    RE::NiPoint3 downRayDir(0, 0, -1);

    bool foundVaulter = false;
    float foundVaultHeight = -10000;

    bool foundLanding = false;
    float foundLandingHeight = 10000;

    for (int i = 0; i < downIterations; i++) {

        float iDist = (float)i * 5;
        
        RE::NiPoint3 downRayStart = playerPos + checkDir * iDist;
        downRayStart.z = fwdRayStart.z;


        float downRayDist = RayCast(downRayStart, downRayDir, headHeight + 100, normalOut, RE::COL_LAYER::kLOS);

        float hitHeight = (fwdRayStart.z - downRayDist) - playerPos.z;

        if (hitHeight > maxVaultHeight) {
            return -1;

        } else if (hitHeight > minVaultHeight && hitHeight < maxVaultHeight) {
            if (hitHeight >= foundVaultHeight) {
                foundVaultHeight = hitHeight;
                foundLanding = false;
            }
            ledgePoint = downRayStart + downRayDir * downRayDist;
            foundVaulter = true;
        } 
        else if (foundVaulter && hitHeight < minVaultHeight) {
            if (hitHeight < foundLandingHeight || foundLanding == false) {
                foundLandingHeight = hitHeight;
            }
            foundLandingHeight = std::min(hitHeight, foundLandingHeight);
            foundLanding = true;
        }
        

    }
     if (foundVaulter && foundLanding && foundLandingHeight < maxElevationIncrease) {
        ledgePoint.z = playerPos.z + foundVaultHeight;

        
        
        //// Calculate horizontal and vertical distances
        //float horizontalDistance = sqrt(pow(ledgePoint.x - playerPos.x, 2) + pow(ledgePoint.y - playerPos.y, 2));
        //float verticalDistance = abs(ledgePoint.z - playerPos.z);

        //// Check if horizontal distance is more than half of the vertical distance
        //if (horizontalDistance > floor(verticalDistance /** 3 / 4*/)) {
        //    //logger::info("Vault too far H:{} V:{}",horizontalDistance, verticalDistance);
        //    return -1;  // Cancel climb if too far horizontally
        //}
        //logger::info("Vaulting=> H:{} V:{}", horizontalDistance, verticalDistance);
        if (!PlayerIsOnStairs())
            //logger::info("Returned Vault");
            return 3;
        
        
    }

    return -1;



}



int GetLedgePoint(RE::TESObjectREFR *vaultMarkerRef, RE::TESObjectREFR *medMarkerRef, RE::TESObjectREFR *highMarkerRef,
                  RE::TESObjectREFR *indicatorRef, bool enableVaulting, bool enableLedges, RE::TESObjectREFR *grabMarkerRef,
                  float backwardOffset = 55.0f) {
    
    // Nullptr check
    if (!indicatorRef || !vaultMarkerRef || !medMarkerRef || !highMarkerRef || !grabMarkerRef) {
        return -1;
    }
    const auto player = RE::PlayerCharacter::GetSingleton();
    const auto playerPos = player->GetPosition();

    // Changed camera angle with player facing direction for 360 climbing

    // Get the player's yaw (Z rotation) in radians
    float playerYaw = player->data.angle.z;  // Player's yaw angle

    // Convert yaw to forward direction vector using sine and cosine
    RE::NiPoint3 playerForwardDir;
    playerForwardDir.x = std::sin(playerYaw);
    playerForwardDir.y = std::cos(playerYaw);
    playerForwardDir.z = 0;  // Player's forward vector is on the XY plane

    // Normalize the player's direction vector
    float playerDirTotal = magnitudeXY(playerForwardDir.x, playerForwardDir.y);
    RE::NiPoint3 playerDirFlat;
    playerDirFlat.x = playerForwardDir.x / playerDirTotal;
    playerDirFlat.y = playerForwardDir.y / playerDirTotal;
    playerDirFlat.z = 0;

    int selectedLedgeType = -1;
    RE::NiPoint3 ledgePoint;

    // Perform ledge check based on player direction
    if (enableVaulting) {
        selectedLedgeType = VaultCheck(ledgePoint, playerDirFlat, 120, 70 * PlayerScale,
                                       static_cast<float>(std::min(40.5, 40.5 * PlayerScale)), 90 * PlayerScale);
        
    }

    if (selectedLedgeType == -1 && enableLedges) {
        selectedLedgeType = LedgeCheck(ledgePoint, playerDirFlat, 40 * PlayerScale, 250 * PlayerScale);
        
    }

    if (selectedLedgeType == -1) {
        return -1;
    }

    // If player's direction is too far away from the ledge point
    if (PlayerVsObjectAngle(ledgePoint) > 80) {
        //logger::info("Player is too far away from ledge");
        return -1;
    }

    // Rotate to face the player's forward direction
    float zAngle = atan2(playerDirFlat.x, playerDirFlat.y);

    // Move the indicator to the player position if needed
    if (indicatorRef->GetParentCell() != player->GetParentCell()) {
        indicatorRef->MoveTo(player->AsReference());
    }

    // Position the indicator above the ledge point, with an offset backward
    RE::NiPoint3 backwardAdjustment = playerDirFlat * backwardOffset * PlayerScale;
    indicatorRef->data.location = ledgePoint /*- backwardAdjustment */+ RE::NiPoint3(0, 0, 5);
    indicatorRef->Update3DPosition(true);
    indicatorRef->data.angle = RE::NiPoint3(0, 0, zAngle);

    RE::TESObjectREFR *ledgeMarker;
    float zAdjust;
    //zAdjust = ceil(playerPos.z - ledgePoint.z);

    // ledge  grab
    if (selectedLedgeType == 5) {
        //logger::info("Selected Grab Ledge");
        ledgeMarker = grabMarkerRef;
        zAdjust = -80 * PlayerScale;
        backwardAdjustment = playerDirFlat */*(backwardOffset-5)*/ 50 * PlayerScale;     // 50 is fine for this
    }
    // Select ledge type
    else if (selectedLedgeType == 1) {
        //logger::info("Selected Med Ledge");
        ledgeMarker = medMarkerRef;
        zAdjust = -155 * PlayerScale;
    } else if (selectedLedgeType == 2) {
        //logger::info("Selected High Ledge");
        ledgeMarker = highMarkerRef;
        zAdjust = -200 * PlayerScale;
    } else {
        //logger::info("Selected Vault");
        ledgeMarker = vaultMarkerRef;
        zAdjust = -60 * PlayerScale;  // default -60
    }


    // Adjust the ledge marker for correct positioning, applying the backward offset
    RE::NiPoint3 adjustedPos;
    adjustedPos.x = ledgePoint.x + playerDirFlat.x - backwardAdjustment.x;
    adjustedPos.y = ledgePoint.y + playerDirFlat.y - backwardAdjustment.y;
    adjustedPos.z = ledgePoint.z + zAdjust;

    if (!ledgeMarker) return -1;

    if (ledgeMarker->GetParentCell() != player->GetParentCell()) {
        ledgeMarker->MoveTo(player->AsReference());
    }

    ledgeMarker->SetPosition(adjustedPos);
    ledgeMarker->data.angle = RE::NiPoint3(0, 0, zAngle);
    

    return selectedLedgeType;
}



int UpdateParkourPoint(RE::StaticFunctionTag *, RE::TESObjectREFR *vaultMarkerRef, RE::TESObjectREFR *medMarkerRef,
                       RE::TESObjectREFR *highMarkerRef, RE::TESObjectREFR *indicatorRef, bool useJumpKey,
                       bool enableVaulting, bool enableLedges, RE::TESObjectREFR *grabMarkerRef) {
    
   
    PlayerScale = GetScale();

    int foundLedgeType = GetLedgePoint(vaultMarkerRef, medMarkerRef, highMarkerRef, indicatorRef, enableVaulting, enableLedges, grabMarkerRef);
    

    if (useJumpKey) {
        if (foundLedgeType >= 0) {
            ToggleJumpingInternal(false);
        } else {
            ToggleJumpingInternal(true);
        }
    }
    if (PlayerIsGrounded() == false || PlayerIsInWater() == true) {
        ToggleJumpingInternal(true);    // Fix jump key getting stuck if next iteration returns -1 after jump key is disabled
        //logger::info("Grounded {} In Water {}", PlayerIsGrounded(), PlayerIsInWater());
        return -1;
    }
    return foundLedgeType;
}


bool PapyrusFunctions(RE::BSScript::IVirtualMachine * vm) { 
    vm->RegisterFunction("SayHello", "SkyClimbPapyrus", SayHello);

    vm->RegisterFunction("ToggleJumping", "SkyClimbPapyrus", ToggleJumping);

    vm->RegisterFunction("EndAnimationEarly", "SkyClimbPapyrus", EndAnimationEarly);

    vm->RegisterFunction("UpdateParkourPoint", "SkyClimbPapyrus", UpdateParkourPoint);

    vm->RegisterFunction("IsClimbKeyDown", "SkyClimbPapyrus", IsClimbKeyDown);

    vm->RegisterFunction("RegisterClimbButton", "SkyClimbPapyrus", RegisterClimbButton);

    vm->RegisterFunction("RegisterClimbDelay", "SkyClimbPapyrus", RegisterClimbDelay);

    vm->RegisterFunction("IsParkourActive", "SkyClimbPapyrus", IsParkourActive);

    return true; 
}

extern "C" DLLEXPORT constinit auto SKSEPlugin_Version = []() {
    SKSE::PluginVersionData v;
    v.PluginVersion({Version::MAJOR, Version::MINOR, Version::PATCH});
    v.PluginName("SkyClimb");
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
    logger::info("Initializing SkyParkour");
    SKSE::GetPapyrusInterface()->Register(PapyrusFunctions); 
    SKSE::GetMessagingInterface()->RegisterListener(MessageEvent);
    
    return true;
}
