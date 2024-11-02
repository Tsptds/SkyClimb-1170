ScriptName SkyClimbPapyrus Hidden

string function SayHello() global native

function ToggleJumping(bool enabled) global native

function EndAnimationEarly(ObjectReference objectRef) global native

int function UpdateParkourPoint(ObjectReference vaultMarkerRef, ObjectReference medMarkerRef, ObjectReference highMarkerRef, ObjectReference indicatorRef, bool useJumpKey, bool enableVaulting, bool enableLedges, ObjectReference grabMarkerRef) global native

function RegisterClimbButton(int DXScanCode) global native

function RegisterClimbDelay(float delay) global native

bool function IsClimbKeyDown() global native