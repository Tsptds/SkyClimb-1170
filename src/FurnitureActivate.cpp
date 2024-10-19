

template <typename Func>
auto WriteFunctionHook(std::uint64_t id, std::size_t byteCopyCount, Func destination) {
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