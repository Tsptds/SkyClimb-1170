//#include "RE/Skyrim.h"
//#include "REL/Relocation.h"
//#include "SKSE/API.h"



bool logSwitch = true;

//RE::INPUT_DEVICE::kKeyboard;  // 0
//RE::INPUT_DEVICE::kMouse;     // 1
//RE::INPUT_DEVICE::kGamepad;   // 2

namespace ButtonStates {
    int8_t DXCODE = -1;
    bool isDown = false;
}

class ButtonEventListener : public RE::BSTEventSink<RE::InputEvent*> {
public:
    static void Register() {
        auto inputManager = RE::BSInputDeviceManager::GetSingleton();
        if (inputManager) {
            inputManager->AddEventSink(new ButtonEventListener());
        }
    }

    RE::BSEventNotifyControl ProcessEvent(RE::InputEvent* const* a_event,
                                          RE::BSTEventSource<RE::InputEvent*>*) override {
        if (!a_event) return RE::BSEventNotifyControl::kContinue;

        for (auto event = *a_event; event; event = event->next) {
            if (const auto buttonEvent = event->AsButtonEvent()) {
                // Check if the event is from a keyboard and is a key press
                //if (buttonEvent->GetDevice() == RE::INPUT_DEVICE::kKeyboard && buttonEvent->IsDown())
                
                const auto dxScanCode = buttonEvent->GetIDCode();  // DX Scan Code
                if (ButtonStates::DXCODE != -1 && dxScanCode == ButtonStates::DXCODE) {
                    
                    // if button is held down more than 2 seconds
                    if (buttonEvent->IsDown() && buttonEvent->HeldDuration() > 2.0f ) {
                        ButtonStates::isDown = true;

                        if (logSwitch)
                            logger::info("Climb key DX code : {}, Input Type: {}", dxScanCode,
                                         buttonEvent->GetDevice());
                    }
                    // immediately let go off the button
                    if (buttonEvent->IsUp()) {
                        ButtonStates::isDown = false;

                        if (logSwitch)
                            logger::info("Climb key DX code : {}, Input Type: {}", dxScanCode,
                                         buttonEvent->GetDevice());
                    }
                    
                    logger::info("Climb key DX code : {}, Input Type: {}", dxScanCode, buttonEvent->GetDevice());
                }
            }
            return RE::BSEventNotifyControl::kContinue;
        }
    }
};