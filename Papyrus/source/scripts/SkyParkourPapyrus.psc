ScriptName SkyParkourPapyrus Hidden

string function SayHello() global native

function ToggleJumping(bool enabled) global native

function EndAnimationEarly(ObjectReference objectRef) global native

int function UpdateParkourPoint(bool useJumpKey, bool enableVaulting, bool enableLedges) global native

function RegisterClimbButton(int DXScanCode) global native

function RegisterClimbDelay(float delay) global native

function RegisterReferences(ObjectReference vaultMarkerRef, ObjectReference lowMarkerRef, ObjectReference medMarkerRef, ObjectReference highMarkerRef, ObjectReference indicatorRef) global native

bool function IsClimbKeyDown() global native

bool function IsParkourActive() global native