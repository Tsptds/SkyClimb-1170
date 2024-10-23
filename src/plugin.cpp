namespace logger = SKSE::log;

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



//RE::NiPoint3 CameraDirInternal() {
//
//    const auto worldCamera = RE::Main::WorldRootCamera();
//
//    
//
//    RE::NiPoint3 output;
//    output.x = worldCamera->world.rotate.entry[0][0];
//    output.y = worldCamera->world.rotate.entry[1][0];
//    output.z = worldCamera->world.rotate.entry[2][0];
//
//    return output;
//}

float getSign(float x) {
    if (x < 0) return -1;
    else return 1;
}

void ToggleJumpingInternal(bool enabled) {
    RE::ControlMap::GetSingleton()->ToggleControls(RE::ControlMap::UEFlag::kJumping, enabled);
}

void ToggleJumping(RE::StaticFunctionTag *, bool enabled) { ToggleJumpingInternal(enabled); }

void EndAnimationEarly(RE::StaticFunctionTag *, RE::TESObjectREFR *objectRef) {
    objectRef->NotifyAnimationGraph("IdleFurnitureExit");
}

    //camera versus head 'to object angle'. Angle between the vectors 'camera to object' and 'player head to object'
//float CameraVsHeadToObjectAngle(RE::NiPoint3 objPoint) {
//    const auto player = RE::PlayerCharacter::GetSingleton();
//
//    RE::NiPoint3 playerToObject = objPoint - (player->GetPosition() + RE::NiPoint3(0, 0, 120));
//
//    playerToObject /= playerToObject.Length();
//
//    RE::NiPoint3 camDir = CameraDirInternal();
//
//    float dot = playerToObject.Dot(camDir);
//
//    const float radToDeg = (float)57.2958;
//
//    return acos(dot) * radToDeg;
//}

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

// Global variable, hate doing this
RE::COL_LAYER lastHitObject;
// switch for layer hit logs
bool logLayer = false;

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
    pickData.rayInput.enableShapeCollectionFilter = false;
    pickData.rayInput.filterInfo = RE::bhkCollisionFilter::GetSingleton()->GetNewSystemGroup() << 16 | SKSE::stl::to_underlying(layerMask);

    if (bhkWorld->PickObject(pickData); pickData.rayOutput.HasHit()) {

        normalOut = pickData.rayOutput.normal;

        uint32_t layerIndex = pickData.rayOutput.rootCollidable->broadPhaseHandle.collisionFilterInfo & 0x7F;

        if (logLayer) logger::info("\nlayer hit: {}", layerIndex);

        //fail if hit a character
        switch (static_cast<RE::COL_LAYER>(layerIndex)) {
            case RE::COL_LAYER::kStatic:
            case RE::COL_LAYER::kCollisionBox:
            case RE::COL_LAYER::kTerrain:
            case RE::COL_LAYER::kGround:
            case RE::COL_LAYER::kProps:
            case RE::COL_LAYER::kClutter:

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

    RE::NiPoint3 groundedRayStart;
    groundedRayStart.x = playerPos.x;
    groundedRayStart.y = playerPos.y;
    groundedRayStart.z = playerPos.z + 128;

    RE::NiPoint3 groundedRayDir(0, 0, -1);

    float groundedRayDist = RayCast(groundedRayStart, groundedRayDir, groundedCheckDist, normalOut, /*false,*/ RE::COL_LAYER::kLOS);

    if (groundedRayDist == groundedCheckDist || groundedRayDist == -1) {
        return false;
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


int LedgeCheck(RE::NiPoint3 &ledgePoint, RE::NiPoint3 checkDir, float minLedgeHeight, float maxLedgeHeight) {

    const auto player = RE::PlayerCharacter::GetSingleton();
    const auto playerPos = player->GetPosition();
    //const auto playerInInterior = player->GetParentCell()->IsInteriorCell();

    float startZOffset = 100;  // how high to start the raycast above the feet of the player
    float playerHeight = 120;  // how much headroom is needed
    float minUpCheck = 100;    // how low can the roof be relative to the raycast starting point?
    float maxUpCheck = (maxLedgeHeight - startZOffset) + 20;  // how high do we even check for a roof?
    float fwdCheck = 8; // how much each incremental forward check steps forward               // Default 10
    int fwdCheckIterations = 15;  // how many incremental forward checks do we make?            // Default 12
    float minLedgeFlatness = 0.5;  // 1 is perfectly flat, 0 is completely perpendicular        // Default 0.5

    RE::hkVector4 normalOut(0, 0, 0, 0);

    // raycast upwards to sky, making sure no roof is too low above us
    RE::NiPoint3 upRayStart;
    upRayStart.x = playerPos.x;
    upRayStart.y = playerPos.y;
    upRayStart.z = playerPos.z + startZOffset;

    RE::NiPoint3 upRayDir(0, 0, 1);

    float upRayDist = RayCast(upRayStart, upRayDir, maxUpCheck, normalOut,/* false,*/ RE::COL_LAYER::kLOS);

    if (upRayDist < minUpCheck) {
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
    float headroomBuffer = 10; // default 10

    RE::NiPoint3 headroomRayStart = ledgePoint + upRayDir * headroomBuffer;

    float headroomRayDist = RayCast(headroomRayStart, upRayDir, playerHeight - headroomBuffer, normalOut, /*false,*/ RE::COL_LAYER::kLOS);

    if (headroomRayDist < playerHeight - headroomBuffer) {
        return -1;
    }
    float ledgePlayerDiff = ledgePoint.z - playerPos.z;
    logger::info("**************\nLedge - player {}", ledgePlayerDiff);
    logger::info("Flatness {}", normalZ);
    if (ledgePlayerDiff > 175) {
        logger::info("Returned High Ledge");
        return 2;
    } else if (ledgePlayerDiff >= 120) {
        logger::info("Returned Med Ledge");
        return 1;
        
    } else {
        
        // Calculate horizontal and vertical distances
        float horizontalDistance = sqrt(pow(ledgePoint.x - playerPos.x, 2) + pow(ledgePoint.y - playerPos.y, 2));
        float verticalDistance = abs(ledgePoint.z - playerPos.z);
        logger::info("Horizontal distance: {} Vertical distance: {}", horizontalDistance, verticalDistance);
        // Check if horizontal distance is more than half of the vertical distance
        if (horizontalDistance > floor(verticalDistance / 2)) { // this greatly prevents climbing into stairs while allowing low height grabs
            logger::info("Too far horizontally");
            return -1;  // Cancel climb if too far horizontally
        }

        logger::info("Returned Grab Ledge");
        
        return 5;
    }
    return -1;
    /*if (ledgePoint.z - playerPos.z < 175) {
        return 1;
    } else {
        return 2;
    }*/
}

int VaultCheck(RE::NiPoint3 &ledgePoint, RE::NiPoint3 checkDir, float vaultLength, float maxElevationIncrease, float minVaultHeight, float maxVaultHeight) {

    const auto player = RE::PlayerCharacter::GetSingleton();
    const auto playerPos = player->GetPosition();

    RE::hkVector4 normalOut(0, 0, 0, 0);

    float headHeight = 120;

    RE::NiPoint3 fwdRayStart;
    fwdRayStart.x = playerPos.x;
    fwdRayStart.y = playerPos.y;
    fwdRayStart.z = playerPos.z + headHeight;

    float fwdRayDist = RayCast(fwdRayStart, checkDir, vaultLength, normalOut, /*false,*/ RE::COL_LAYER::kLOS);

    if (fwdRayDist < vaultLength && lastHitObject == RE::COL_LAYER::kTerrain) {
        return -1;
    }

    // Backward ray to check if an object is blocking behind the vaultable surface
    RE::NiPoint3 backwardRayStart = fwdRayStart + checkDir * (fwdRayDist - 5) + RE::NiPoint3(0,0,5);  // Start the ray from the hit point with a z offset, and a starting offset

    // Check for any object within a small distance behind the vaultable object
    float backwardRayDist = RayCast(backwardRayStart, checkDir, 50.0f, normalOut, RE::COL_LAYER::kLOS);
    if (backwardRayDist < 50.0f && backwardRayDist > 0) {
        // An object was found behind the vaultable surface, so cancel the vault
        return -1;
    }

    int downIterations = /*(int)std::floor(vaultLength / 5.0f)*/ 24;       //Default 5.0f

    RE::NiPoint3 downRayDir(0, 0, -1);

    bool foundVaulter = false;
    float foundVaultHeight = -10000;

    bool foundLanding = false;
    float foundLandingHeight = 10000;

    for (int i = 0; i < downIterations; i++) {

        float iDist = (float)i * 5;
        
        RE::NiPoint3 downRayStart = playerPos + checkDir * iDist;
        downRayStart.z = fwdRayStart.z;


        float downRayDist = RayCast(downRayStart, downRayDir, headHeight + 100, normalOut, /*false,*/ RE::COL_LAYER::kLOS);

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
    /*if (!foundLanding && playerPos.z - ledgePoint.z > 0) {
        return 5;
    }
    else*/ if (foundVaulter && foundLanding && foundLandingHeight < maxElevationIncrease) {
        ledgePoint.z = playerPos.z + foundVaultHeight;

        // Always cut off animation for smoother vaults, removed 4 in papyrus and replaced with 3 too
        /*if (foundLandingHeight < -10) {
            return 4;
        }*/
        
        // Calculate horizontal and vertical distances
        float horizontalDistance = sqrt(pow(ledgePoint.x - playerPos.x, 2) + pow(ledgePoint.y - playerPos.y, 2));
        float verticalDistance = abs(ledgePoint.z - playerPos.z);

        // Check if horizontal distance is more than half of the vertical distance
        if (horizontalDistance > floor(verticalDistance)) {
            return -1;  // Cancel climb if too far horizontally
        }

        return 3;
        
        
    }

    return -1;



}



int GetLedgePoint(RE::TESObjectREFR *vaultMarkerRef, RE::TESObjectREFR *medMarkerRef, RE::TESObjectREFR *highMarkerRef,
                  RE::TESObjectREFR *indicatorRef, bool enableVaulting, bool enableLedges, RE::TESObjectREFR *grabMarkerRef,
                  float backwardOffset = 55.0f) {
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
        selectedLedgeType = VaultCheck(ledgePoint, playerDirFlat, 130, 70, 28, 90);
        //selectedLedgeType = LedgeCheck(ledgePoint, playerDirFlat, 60, 250);    // defaults 110, 250 
    }

    if (selectedLedgeType == -1 && enableLedges) {
        selectedLedgeType = LedgeCheck(ledgePoint, playerDirFlat, 30, 250);
        //// prevent vaulting over to clip out-of-bounds
        //if (player->GetParentCell()->IsInteriorCell())
        //    return -1;
        //else
            //selectedLedgeType = VaultCheck(ledgePoint, playerDirFlat, 130, 50, 30, 60);   // defaults 120 10 50 100    // old values 130, 85, 35, 115
        // Replaced max elevation increase with 30 from 10, min vault with 35 from 50, max vault with 109 from 100
    }

    if (selectedLedgeType == -1) {
        return -1;
    }

    // If player's direction is too far away from the ledge point
    if (PlayerVsObjectAngle(ledgePoint) > 80) {
        logger::info("Player is too far away from ledge");
        return -1;
    }

    // Rotate to face the player's forward direction
    float zAngle = atan2(playerDirFlat.x, playerDirFlat.y);

    // Move the indicator to the player position if needed
    if (indicatorRef->GetParentCell() != player->GetParentCell()) {
        indicatorRef->MoveTo(player->AsReference());
    }

    // Position the indicator above the ledge point, with an offset backward
    RE::NiPoint3 backwardAdjustment = playerDirFlat * backwardOffset;
    indicatorRef->data.location = ledgePoint /*- backwardAdjustment */+ RE::NiPoint3(0, 0, 5);
    indicatorRef->Update3DPosition(true);
    indicatorRef->data.angle = RE::NiPoint3(0, 0, zAngle);

    RE::TESObjectREFR *ledgeMarker;
    float zAdjust;
    //zAdjust = ceil(playerPos.z - ledgePoint.z);

    // ledge  grab
    if (selectedLedgeType == 5) {
        ledgeMarker = grabMarkerRef;
        zAdjust = -80;
        backwardAdjustment = playerDirFlat */*(backwardOffset-5)*/ 50;     // 50 is fine for this
    }
    // Select ledge type
    else if (selectedLedgeType == 1) {
        ledgeMarker = medMarkerRef;
        zAdjust = -155;
    } else if (selectedLedgeType == 2) {
        ledgeMarker = highMarkerRef;
        zAdjust = -200;
    } else {
        ledgeMarker = vaultMarkerRef;
        zAdjust = -60;  // default -60
    }


    // Adjust the ledge marker for correct positioning, applying the backward offset
    RE::NiPoint3 adjustedPos;
    adjustedPos.x = ledgePoint.x + playerDirFlat.x - backwardAdjustment.x;
    adjustedPos.y = ledgePoint.y + playerDirFlat.y - backwardAdjustment.y;
    adjustedPos.z = ledgePoint.z + zAdjust;

    if (ledgeMarker->GetParentCell() != player->GetParentCell()) {
        ledgeMarker->MoveTo(player->AsReference());
    }

    ledgeMarker->SetPosition(adjustedPos);
    ledgeMarker->data.angle = RE::NiPoint3(0, 0, zAngle);
    

    return selectedLedgeType;
}

// int GetLedgePoint(RE::TESObjectREFR *vaultMarkerRef, RE::TESObjectREFR *medMarkerRef, RE::TESObjectREFR
// *highMarkerRef, RE::TESObjectREFR *indicatorRef, bool enableVaulting, bool enableLedges) {
//
//
//     const auto player = RE::PlayerCharacter::GetSingleton();
//     const auto playerPos = player->GetPosition();
//
//     RE::NiPoint3 cameraDir = CameraDirInternal();
//
//     float cameraDirTotal = magnitudeXY(cameraDir.x, cameraDir.y);
//     RE::NiPoint3 cameraDirFlat;
//     cameraDirFlat.x = cameraDir.x / cameraDirTotal;
//     cameraDirFlat.y = cameraDir.y / cameraDirTotal;
//     cameraDirFlat.z = 0;
//
//     int selectedLedgeType = -1;
//     RE::NiPoint3 ledgePoint;
//
//     if (enableLedges) {
//         selectedLedgeType = LedgeCheck(ledgePoint, cameraDirFlat, 110, 250);
//     }
//
//     if (selectedLedgeType == -1) {
//
//         if (enableVaulting) {
//             selectedLedgeType = VaultCheck(ledgePoint, cameraDirFlat, 120, 10, 50, 100);
//         }
//
//         if (selectedLedgeType == -1)
//         {
//             return -1;
//         }
//     }
//
//
//     // if camera facing too far away from ledgepoint
//     if (CameraVsHeadToObjectAngle(ledgePoint) > 80) {
//         return -1;
//     }
//
//
//     // rotate to face camera
//     float zAngle = atan2(cameraDirFlat.x, cameraDirFlat.y);
//
//
//     //it seems we have to MoveTo first in order to get the references into the same cell
//
//     if (indicatorRef->GetParentCell() != player->GetParentCell())
//     {
//         indicatorRef->MoveTo(player->AsReference());
//     }
//
//     indicatorRef->data.location = ledgePoint + RE::NiPoint3(0,0,5);
//     indicatorRef->Update3DPosition(true);
//
//     indicatorRef->data.angle = RE::NiPoint3(0, 0, zAngle);
//
//     RE::TESObjectREFR *ledgeMarker;
//
//     float zAdjust;
//     float toCameraAdjust;
//
//
//     //pick a ledge type
//
//
//     if (selectedLedgeType == 1) {
//         ledgeMarker = medMarkerRef;
//         zAdjust = -155;
//         toCameraAdjust = -50;
//     }
//     else if (selectedLedgeType == 2) {
//         ledgeMarker = highMarkerRef;
//         zAdjust = -200;
//         toCameraAdjust = -50;
//     }
//     else {
//         ledgeMarker = vaultMarkerRef;
//         zAdjust = -60;
//         toCameraAdjust = -80;
//     }
//
//     // adjust the EVG marker for correct positioning
//     RE::NiPoint3 adjustedPos;
//     adjustedPos.x = ledgePoint.x + cameraDirFlat.x * toCameraAdjust;
//     adjustedPos.y = ledgePoint.y + cameraDirFlat.y * toCameraAdjust;
//     adjustedPos.z = ledgePoint.z + zAdjust;
//
//     if (ledgeMarker->GetParentCell() != player->GetParentCell()) {
//         ledgeMarker->MoveTo(player->AsReference());
//     }
//
//     ledgeMarker->SetPosition(adjustedPos);
//
//     ledgeMarker->data.angle = RE::NiPoint3(0, 0, zAngle);
//
//
//
//     return selectedLedgeType;
//
//
// }

int UpdateParkourPoint(RE::StaticFunctionTag *, RE::TESObjectREFR *vaultMarkerRef, RE::TESObjectREFR *medMarkerRef,
                       RE::TESObjectREFR *highMarkerRef, RE::TESObjectREFR *indicatorRef, bool useJumpKey,
                       bool enableVaulting, bool enableLedges, RE::TESObjectREFR *grabMarkerRef) {
    
    //if (PlayerIsGrounded() == false || PlayerIsInWater() == true) {
    //    return -1;
    //}

    int foundLedgeType = GetLedgePoint(vaultMarkerRef, medMarkerRef, highMarkerRef, indicatorRef, enableVaulting, enableLedges, grabMarkerRef);
    

    if (useJumpKey) {
        if (foundLedgeType >= 0) {
            ToggleJumpingInternal(false);
        } else {
            ToggleJumpingInternal(true);
        }
    }
    if (PlayerIsGrounded() == false || PlayerIsInWater() == true) {
        // ToggleJumpingInternal(true);    // Fix jump key getting stuck if next iteration returns -1 after jump key is disabled
        return -1;
    }
    return foundLedgeType;
}


bool PapyrusFunctions(RE::BSScript::IVirtualMachine * vm) { 
    vm->RegisterFunction("SayHello", "SkyClimbPapyrus", SayHello);

    vm->RegisterFunction("ToggleJumping", "SkyClimbPapyrus", ToggleJumping);

    vm->RegisterFunction("EndAnimationEarly", "SkyClimbPapyrus", EndAnimationEarly);

    vm->RegisterFunction("UpdateParkourPoint", "SkyClimbPapyrus", UpdateParkourPoint);

    return true; 
}

extern "C" DLLEXPORT constinit auto SKSEPlugin_Version = []() {
    SKSE::PluginVersionData v;
    v.PluginVersion({Version::MAJOR, Version::MINOR, Version::PATCH});
    v.PluginName("SkyClimb");
    v.AuthorName("Sokco");
    v.UsesAddressLibrary();
    v.UsesUpdatedStructs();
    v.CompatibleVersions({SKSE::RUNTIME_1_6_1130, 
        {(unsigned short)1U, (unsigned short)6U, (unsigned short)1170U, (unsigned short)0U}});

    return v;
}();

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface *a_skse) {
    SetupLog();

    SKSE::Init(a_skse);
    logger::info("SkyClimb Papyrus Started!");
    SKSE::GetPapyrusInterface()->Register(PapyrusFunctions);    

    return true;
}

/*
SKSEPluginLoad(const SKSE::LoadInterface *skse) {
    SKSE::Init(skse);

    //////// PLUGIN START /////////


    SetupLog();
    logger::info("SkyClimb Papyrus Started!");

    SKSE::GetPapyrusInterface()->Register(PapyrusFunctions);


    //auto *eventSink = new OurEventSink();

    //SKSE::GetCameraEventSource()->AddEventSink(eventSink);

    //////// PLUGIN END /////////

    return true;
}
*/