ScriptName SkyClimbPapyrus Hidden

string function SayHello() global native

function ToggleJumping(bool enabled) global native

function EndAnimationEarly(ObjectReference objectRef) global native

int function UpdateParkourPoint(ObjectReference ledgeGrabMarkerRef, ObjectReference vaultMarkerRef, ObjectReference medMarkerRef, ObjectReference highMarkerRef, ObjectReference indicatorRef, bool useJumpKey, bool enableVaulting, bool enableLedges) global native