#pragma once
#include "References.cpp"
bool logSwitch = false;


namespace ButtonStates {
    
    float lastKeyDownTime = 0.0f;  // Track time of last valid press

    int32_t DXCODE = 0;
    bool isDown = false;

    std::unordered_map<int32_t, int32_t> xinputToCKMap = {
        // Mouse
        {2, 258},  // Mouse middle
        {3, 259},  // M4
        {4, 260},  // M5
        // Not natively supported
        //{5, 261},  // M6
        //{6, 262},  // M7
        //{7, 263},  // ? tf is 8th button

        // These have no button up events, input gets stuck on wheel up down
        //{8, 264},  // Wheel Up
        //{9, 265},  // Wheel Down
        
        // Gamepad
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

    int32_t MapToCKIfPossible(int32_t dxcode) {
        
        auto it = xinputToCKMap.find(dxcode);
        if (it != xinputToCKMap.end()) {
            //logger::info("Alt. CK input found, mapping {}", it->second);
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
                
                auto dxScanCode = static_cast<int32_t>(buttonEvent->GetIDCode());  // DX Scan Code
                //logger::info("DX code : {}, Input Type: {}", dxScanCode, buttonEvent->GetDevice());
                
                // Unless there's a valid key code, skip the mapping
                if (ButtonStates::DXCODE == 0) continue;

                // Convert Xinput codes to creation kit versions
                if (buttonEvent->GetDevice() == RE::INPUT_DEVICE::kGamepad) {
                    
                    const auto ckMapped = ButtonStates::xinputToCKMap[dxScanCode];
                    
                    if(logSwitch) 
                        logger::info("Gamepad: Xinput: {} -> CK Map: {}", dxScanCode, ckMapped);
                    dxScanCode = ckMapped;
                }
                else if (buttonEvent->GetDevice() == RE::INPUT_DEVICE::kMouse) {
                    const auto ckMapped = ButtonStates::xinputToCKMap[dxScanCode];

                    if (logSwitch) 
                        logger::info("Mouse: Xinput: {} -> CK Map: {}", dxScanCode, ckMapped);
                    dxScanCode = ckMapped;
                }

                if (dxScanCode != ButtonStates::DXCODE) continue;

                // Continue if there's a defined dxcode 
                if (dxScanCode == ButtonStates::DXCODE) {
                    if (buttonEvent->IsDown()) {
                        // if (logSwitch) logger::info("Holding Climb key, DX code : {}, Input Type: {}", dxScanCode, buttonEvent->GetDevice());
                    }
                    // if button is held down more than x seconds
                    if (buttonEvent->IsPressed()) {
                        if (ButtonStates::lastKeyDownTime + ModSettings::climbDelay <=
                            buttonEvent->HeldDuration()) {
                            ButtonStates::isDown = true;
                            ButtonStates::lastKeyDownTime = buttonEvent->HeldDuration();  // Update time of last press
                        }

                        
                    }
                    // immediately let go off the button
                    else /*if (!buttonEvent->IsHeld())*/ {
                        ButtonStates::isDown = false;
                        ButtonStates::lastKeyDownTime = 0.0f;  // Reset time on release

                        //if (logSwitch) logger::info("Climb key released, DX code : {}, Input Type: {}", dxScanCode, buttonEvent->GetDevice());
                    }
                }
            }
            //logger::info("Climb Allowed: {}", ButtonStates::isDown);
            
        }
        // DON'T SKIP ANY INPUTS, THIS GOES AFTER FOR LOOP. OTHERWISE BREAKS OTHER INPUTS
        return RE::BSEventNotifyControl::kContinue;
    }
};