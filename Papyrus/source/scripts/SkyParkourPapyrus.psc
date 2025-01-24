ScriptName SkyParkourPapyrus Hidden

int function UpdateParkourPoint(bool useJumpKey, bool enableVaulting, bool enableLedges) global native

function RegisterClimbButton(int DXScanCode) global native

function RegisterClimbDelay(float delay) global native

function RegisterReferences(ObjectReference vaultMarkerRef, ObjectReference lowMarkerRef, ObjectReference medMarkerRef, ObjectReference highMarkerRef, ObjectReference vaultActivatorRef, ObjectReference lowActivatorRef, ObjectReference medActivatorRef, ObjectReference highActivatorRef, ObjectReference indicatorRef) global native

function RegisterStaminaDamage(bool enabled, float Stamina_Damage) global native

function ToggleJumping(bool enabled) global native