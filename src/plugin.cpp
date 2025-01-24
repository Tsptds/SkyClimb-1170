#include "ButtonListener.cpp"
#include "Util.cpp"
#include "References.h"
#include "Climb_Utility.cpp"
#include "Climbing.cpp"

namespace logger = SKSE::log;


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
    ModSettings::climbDelay = delay;
    logger::info("Delay Registered {}", ModSettings::climbDelay);
}

void RegisterStaminaDamage(RE::StaticFunctionTag *, bool enabled, float damage) {
    ModSettings::Enable_Stamina_Consumption = enabled;
    ModSettings::Stamina_Damage = damage;
    logger::info("Stamina Consumption Updated {} {}", ModSettings::Enable_Stamina_Consumption, ModSettings::Stamina_Damage);
}

void RegisterReferences(RE::StaticFunctionTag *, RE::TESObjectREFR *vaultMarkerRef, RE::TESObjectREFR *lowMarkerRef, 
                        RE::TESObjectREFR *medMarkerRef, RE::TESObjectREFR *highMarkerRef,
                        RE::TESObjectREFR *vaultActivatorRef, RE::TESObjectREFR *lowActivatorRef,
                        RE::TESObjectREFR *medActivatorRef, RE::TESObjectREFR *highActivatorRef,
                        RE::TESObjectREFR *indicatorRef) {

    GameReferences::vaultMarkerRef = vaultMarkerRef;
    GameReferences::lowMarkerRef = lowMarkerRef;
    GameReferences::medMarkerRef = medMarkerRef;
    GameReferences::highMarkerRef = highMarkerRef;

    GameReferences::vaultActivatorRef = vaultActivatorRef;
    GameReferences::lowActivatorRef = lowActivatorRef;
    GameReferences::medActivatorRef = medActivatorRef;
    GameReferences::highActivatorRef = highActivatorRef;

    GameReferences::indicatorRef = indicatorRef;
}


bool PapyrusFunctions(RE::BSScript::IVirtualMachine * vm) {

    vm->RegisterFunction("ToggleJumping", "SkyParkourPapyrus", ToggleJumping);

    //vm->RegisterFunction("EndAnimationEarly", "SkyParkourPapyrus", EndAnimationEarly);

    vm->RegisterFunction("UpdateParkourPoint", "SkyParkourPapyrus", UpdateParkourPoint);

    //vm->RegisterFunction("IsClimbKeyDown", "SkyParkourPapyrus", IsClimbKeyDown);

    vm->RegisterFunction("RegisterClimbButton", "SkyParkourPapyrus", RegisterClimbButton);

    vm->RegisterFunction("RegisterClimbDelay", "SkyParkourPapyrus", RegisterClimbDelay);

    vm->RegisterFunction("RegisterStaminaDamage", "SkyParkourPapyrus", RegisterStaminaDamage);

    vm->RegisterFunction("RegisterReferences", "SkyParkourPapyrus", RegisterReferences);

    /*vm->RegisterFunction("IsParkourActive", "SkyParkourPapyrus", IsParkourActive);*/

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
