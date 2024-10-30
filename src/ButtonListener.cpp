//#include "RE/Skyrim.h"
//#include "REL/Relocation.h"
//#include "SKSE/API.h"



bool logSwitch = false;



//RE::INPUT_DEVICE::kKeyboard;  // 0
//RE::INPUT_DEVICE::kMouse;     // 1
//RE::INPUT_DEVICE::kGamepad;   // 2

// Check if the event is from a keyboard and is a key press
// if (buttonEvent->GetDevice() == RE::INPUT_DEVICE::kKeyboard && buttonEvent->IsDown())

namespace ButtonStates {
    int32_t DXCODE = -1;
    bool isDown = false;
    float lastKeyDownTime = 0.0f;      // Track time of last valid press
    const float debounceDelay = 0.3f;  // Set debounce delay

    std::unordered_map<int32_t, int32_t> xinputToCKMap = {
        {0x0001, 266},  // DPAD_UP
        {0x0002, 267},  // DPAD_DOWN
        {0x0004, 268},  // DPAD_LEFT
        {0x0008, 269},  // DPAD_RIGHT
        {0x0010, 270},  // START
        {0x0020, 271},  // BACK
        {0x0040, 272},  // LEFT_THUMB
        {0x0080, 273},  // RIGHT_THUMB
        {0x0100, 274},  // LEFT_SHOULDER
        {0x0200, 275},  // RIGHT_SHOULDER
        {0x1000, 276},  // A
        {0x2000, 277},  // B
        {0x4000, 278},  // X
        {0x8000, 279}   // Y
    };

    int32_t MapToGamepadIfPossible(int32_t dxcode) {
        
        auto it = xinputToCKMap.find(dxcode);
        if (it != xinputToCKMap.end()) {
            logger::info("Gamepad input found, mapping {}", it->second);
            return it->second;
        }
        return dxcode;  // Return default value if key not found
    }
}

class ButtonEventListener : public RE::BSTEventSink<RE::InputEvent*> {

private:
    ButtonEventListener() = default;
    ButtonEventListener(const ButtonEventListener&) = delete;
    ButtonEventListener& operator=(const ButtonEventListener&) = delete;

public:
    
    static ButtonEventListener* GetSingleton() {
        static ButtonEventListener instance;
        return &instance;
    }


    
    static void Register() {
        auto inputManager = RE::BSInputDeviceManager::GetSingleton();
        if (inputManager) {
            inputManager->AddEventSink(ButtonEventListener::GetSingleton());
            logger::info("Registered Button Listener");
        }
    }

    RE::BSEventNotifyControl ProcessEvent(RE::InputEvent* const* a_event,
                                          RE::BSTEventSource<RE::InputEvent*>*) override {
        if (!a_event) return RE::BSEventNotifyControl::kContinue;

        for (auto event = *a_event; event; event = event->next) {
            if (const auto buttonEvent = event->AsButtonEvent()) {
                
                auto dxScanCode = buttonEvent->GetIDCode();  // DX Scan Code
                //logger::info("DX code : {}, Input Type: {}", dxScanCode, buttonEvent->GetDevice());
                if (dxScanCode != ButtonStates::DXCODE) continue;
                // Unless there's a valid key code, don't check for inputs (Happens when papyrus calls SkyClimbPapyrus.RegisterClimbButton)
                if (ButtonStates::DXCODE == -1) return RE::BSEventNotifyControl::kContinue;

                // Convert Xinput codes to creation kit versions
                if (buttonEvent->GetDevice() == RE::INPUT_DEVICE::kGamepad) {
                    
                    const auto ckMapped = ButtonStates::xinputToCKMap[dxScanCode];
                    
                    if(logSwitch) 
                        logger::info("Gamepad: Xinput: {} -> CK Map: {}", dxScanCode, ckMapped);
                    dxScanCode = ckMapped;
                }

                // Continue if there's a defined dxcode 
                if (dxScanCode == ButtonStates::DXCODE) {
                    if (buttonEvent->IsDown()) {
                        // if (logSwitch)
                        logger::info("Holding Climb key, DX code : {}, Input Type: {}", dxScanCode,
                                     buttonEvent->GetDevice());
                    }
                    // if button is held down more than x seconds
                    if (buttonEvent->IsPressed()) {
                        if (ButtonStates::lastKeyDownTime + ButtonStates::debounceDelay <=
                            buttonEvent->HeldDuration()) {
                            ButtonStates::isDown = true;
                            ButtonStates::lastKeyDownTime = buttonEvent->HeldDuration();  // Update time of last press
                        }

                        
                    }
                    // immediately let go off the button
                    if (buttonEvent->IsUp()) {
                        ButtonStates::isDown = false;
                        ButtonStates::lastKeyDownTime = 0.0f;  // Reset time on release

                        //if (logSwitch)
                            logger::info("Climb key released, DX code : {}, Input Type: {}", dxScanCode,
                                         buttonEvent->GetDevice());
                    }
                }
            }
            //logger::info("Climb Allowed: {}", ButtonStates::isDown);
            return RE::BSEventNotifyControl::kContinue;
        }
    }
};