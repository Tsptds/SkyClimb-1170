Scriptname SkyClimbQuestScript extends Quest  

bool canParkour = false
;bool couldParkourLastFrame = false

bool climbStarted = false

; bool property holdingKey auto
int parkourType = -1

;float lastParkourPosX
;float lastParkourPosY
;float lastParkourPosZ
Actor property playerRef auto

function OnInit()

	;string hello = SkyClimbPapyrus.SayHello()
	
	;Debug.MessageBox(hello)
	
	Maintenance()
	
endFunction

function Maintenance()

	UpdateRefs(false)

	;lastParkourPosX = 0
	;lastParkourPosY = 0
	;lastParkourPosZ = 0
	
	;couldParkourLastFrame = false
	canParkour = false
	climbStarted = false
	
	parkourType = -1
	indicatorRef.Disable()
	
	
	UnregisterForAllKeys()
	; If UseJumpKey
	; 	RegisterForKey(Input.GetMappedKey("Jump")) ; jump
	; Else
	; 	RegisterForKey(ClimbKey)
	; EndIf
	
	If UseJumpKey
		SkyClimbPapyrus.RegisterClimbButton(Input.GetMappedKey("Jump"))
	Else
		SkyClimbPapyrus.RegisterClimbButton(ClimbKey)
	EndIf

	RegisterForSingleUpdate(0.05)
	
	

endFunction

Event OnLocationChange(Location akOldLoc, Location akNewLoc)
	UpdateRefs(false)
EndEvent

bool function ParkourActive()
	;return playerRef.GetSitState() == 0 && Utility.IsInMenuMode() == false ;&& vaultMarkerRef.IsFurnitureInUse() == false && medMarkerRef.IsFurnitureInUse() == false && highMarkerRef.IsFurnitureInUse() == false
	return playerRef.GetSleepState() == 0 && playerRef.GetSitState() == 0 && Utility.IsInMenuMode() == false && playerRef.IsWeaponDrawn() == false
	
endFunction

function UpdateRefs(bool forceUpdateLinks)

	bool updateLinks = false
	; Ledge Grab
	if(grabMarkerRef == none)
		grabMarkerRef = playerRef.PlaceAtMe(grabMarkerProp, 1, true, false)
		updateLinks = true
	endIf

	if(grabActivatorRef == none)
		grabActivatorRef = playerRef.PlaceAtMe(grabActivatorProp, 1, true, false)
		updateLinks = true
	endIf

	if(vaultActivatorRef == none)
		vaultActivatorRef = playerRef.PlaceAtMe(vaultActivatorProp, 1, true, false)
		updateLinks = true
	endIf
	
	if(vaultMarkerRef == none)
		vaultMarkerRef = playerRef.PlaceAtMe(vaultMarkerProp, 1, true, false)
		updateLinks = true
	endif

	if(medActivatorRef == none)
		medActivatorRef = playerRef.PlaceAtMe(medActivatorProp, 1, true, false)
		updateLinks = true
	endIf
	
	if(medMarkerRef == none)
		medMarkerRef = playerRef.PlaceAtMe(medMarkerProp, 1, true, false)
		updateLinks = true
	endif
	
	if(highActivatorRef == none)
		highActivatorRef = playerRef.PlaceAtMe(highActivatorProp, 1, true, false)
		updateLinks = true
	endIf
	
	if(highMarkerRef == none)
		highMarkerRef = playerRef.PlaceAtMe(highMarkerProp, 1, true, false)
		updateLinks = true
	endif
	
	if updateLinks || forceUpdateLinks
		PO3_SKSEFunctions.SetLinkedRef(vaultActivatorRef, vaultMarkerRef)
		PO3_SKSEFunctions.SetLinkedRef(medActivatorRef, medMarkerRef)
		PO3_SKSEFunctions.SetLinkedRef(highActivatorRef, highMarkerRef)
		
		PO3_SKSEFunctions.SetLinkedRef(grabActivatorRef, grabMarkerRef)
	endIf

	if(indicatorRef == none)
		indicatorRef = playerRef.PlaceAtMe(indicatorObject, 1, true, false)
	endif

endFunction

Event OnUpdate()

	if !ParkourActive()
		if canParkour
			canParkour = false
			indicatorRef.Disable()
		endif
		;SkyClimbPapyrus.ToggleJumping(true)
		RegisterForSingleUpdate(0.05)
		return
	endif


	; UpdateRefs(true)
	; couldParkourLastFrame = canParkour

	; if climbStarted == false && ParkourActive()
	if !climbStarted
	
		parkourType = SkyClimbPapyrus.UpdateParkourPoint(vaultMarkerRef, medMarkerRef, highMarkerRef, indicatorRef, UseJumpKey, EnableVaulting, EnableLedges, grabMarkerRef)
	
		if parkourType >= 0
			
			if canParkour == false
				canParkour = true
				indicatorRef.Enable()
			endif

		else
			if canParkour == true
				canParkour = false
				indicatorRef.Disable()
				playerRef.SetAnimationVariableBool("IsInFurniture", false) ; Clear the furniture state
			endif
			
			;keep em disabled
			;vaultMarkerRef.Disable()
			;medMarkerRef.Disable()
			;highMarkerRef.Disable()
			;grabMarkerRef.Disable()
		endif
	 endif

	; if UseJumpKey
    ;     holdingKey = Input.IsKeyPressed(Input.GetMappedKey("Jump"))
    ; else
    ;     holdingKey = Input.IsKeyPressed(ClimbKey)
    ; endif

	if SkyClimbPapyrus.IsClimbKeyDown() ;&& ParkourActive()
		KeepClimbing()
	endif
	RegisterForSingleUpdate(0.05)

EndEvent

; Event OnKeyUp(Int KeyCode, Float HoldTime)
; 	holdingKey = false
; EndEvent

; Event OnKeyDown(Int KeyCode)
; 	holdingKey = true
; EndEvent

function KeepClimbing()
	; if climbStarted == false && couldParkourLastFrame && canParkour && parkourType >= 0 && ParkourActive()		;Default couldParkourLastFrame
	if climbStarted == false && canParkour
		;Actor playerRef = playerRef

		;playerRef.SetAnimationVariableBool("bInJumpState", false)
	
		climbStarted = true
		; holdingKey = false

		if parkourType == 5
			grabMarkerRef.Enable()
			Utility.Wait(0.01)
			grabActivatorRef.Activate(playerRef)
			Utility.Wait(0.05)
			
			;climbStarted = false
		elseif parkourType == 1
			medMarkerRef.Enable()
			Utility.Wait(0.01)
			medActivatorRef.Activate(playerRef)
			Utility.Wait(0.05)

			;climbStarted = false
		
		elseif parkourType == 2
			highMarkerRef.Enable()
			Utility.Wait(0.01)
			highActivatorRef.Activate(playerRef)
			Utility.Wait(0.05)
			
		elseif parkourType == 3
			vaultMarkerRef.Enable()
			Utility.Wait(0.01)
			vaultActivatorRef.Activate(playerRef)
			Utility.Wait(1.1)
			vaultMarkerRef.Disable()

			SkyClimbPapyrus.EndAnimationEarly(playerRef)
			;climbStarted = false

		endif

		climbStarted = false
	endIf

endfunction

ObjectReference Property highMarkerRef Auto Hidden
ObjectReference Property medMarkerRef Auto Hidden
ObjectReference Property vaultMarkerRef Auto Hidden
ObjectReference Property highActivatorRef Auto Hidden
ObjectReference Property medActivatorRef Auto Hidden
ObjectReference Property vaultActivatorRef Auto Hidden

ObjectReference Property grabMarkerRef auto Hidden
ObjectReference Property grabActivatorRef auto Hidden

Activator Property highActivatorProp Auto
Activator Property medActivatorProp Auto
Activator Property vaultActivatorProp Auto
Furniture Property highMarkerProp Auto
Furniture Property medMarkerProp Auto
Furniture Property vaultMarkerProp Auto

Furniture Property grabMarkerProp auto
Activator Property grabActivatorProp auto

ObjectReference Property indicatorRef Auto Hidden
MiscObject Property indicatorObject Auto

Bool Property EnableLedges Auto
Bool Property EnableVaulting Auto
Bool Property UseJumpKey Auto
Int Property ClimbKey Auto
