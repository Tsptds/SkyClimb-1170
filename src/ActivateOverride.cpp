#include "REL/Relocation.h"
#include "ActivateOverride.h"
#include "xbyak/xbyak.h"


bool PlayerCanActivateFurniture::Install() {
    SKSE::AllocTrampoline(14);

    auto& trampoline = SKSE::GetTrampoline();
    logger::info("Installing Furniture Activate Override");
    func = WriteFunctionHook(17420, 11, &thunk);
    REL::Relocation<std::uintptr_t> canActivateFurnitureTarget{RELOCATION_ID(17034, 17420)};  // Obtained from PO3 tweaks; SE 17034, AE 17420
    

    //func = trampoline.write_call<5>(canActivateFurnitureTarget.address(), &thunk);
    return true;
}

bool PlayerCanActivateFurniture::thunk(RE::PlayerCharacter* a_player, RE::TESObjectREFR* a_furniture) {
    bool response = func(a_player, a_furniture);
    logger::info("function called");
    if (!response && a_player->IsInMidair()) {
        logger::info("player is midair and main function returned false, making that true");
        response = true;
    }
    return response;
}

    template <typename Func>
auto WriteFunctionHook(std::uint64_t id, std::size_t byteCopyCount, Func destination) {
    auto& TRAMPOLINE = SKSE::GetTrampoline();
    const REL::Relocation target{REL::ID(id)};
    struct XPatch : Xbyak::CodeGenerator {
        using ull = unsigned long long;
        using uch = unsigned char;
        XPatch(std::uintptr_t originalFuncAddr, ull originalByteLength, ull newByteLength)
            : Xbyak::CodeGenerator(originalByteLength + newByteLength,
                                   TRAMPOLINE.allocate(originalByteLength + newByteLength)) {
            auto byteAddr = reinterpret_cast<uch*>(originalFuncAddr);
            for (ull i = 0; i < originalByteLength; i++) db(*byteAddr++);
            jmp(qword[rip]);
            dq(ull(byteAddr));
        }
    };
    XPatch patch(target.address(), byteCopyCount, 20);
    patch.ready();
    auto patchSize = patch.getSize();
    TRAMPOLINE.write_branch<5>(target.address(), destination);
    auto alloc = TRAMPOLINE.allocate(patchSize);
    memcpy(alloc, patch.getCode(), patchSize);
    return reinterpret_cast<std::uintptr_t>(alloc);
}