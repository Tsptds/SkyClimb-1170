Scriptname SkyClimbQuestScript extends Quest  

bool property canParkour auto
bool property couldParkourLastFrame auto

bool property climbStarted auto

bool property holdingKey auto
int property parkourType auto

;float lastParkourPosX
;float lastParkourPosY
;float lastParkourPosZ
Actor property playerRef auto

Event OnInit()

	;string hello = SkyClimbPapyrus.SayHello()
	
	;Debug.MessageBox(hello)
	
	;Maintenance()
EndEvent

function Maintenance()
	;Debug.MessageBox("Maintenance")
	UpdateRefs()

	;lastParkourPosX = 0
	;lastParkourPosY = 0
	;lastParkourPosZ = 0
	
	;couldParkourLastFrame = false
	canParkour = false
	climbStarted = false
	
	parkourType = -1
	indicatorRef.Disable()
	vaultMarkerRef.Disable()
	medMarkerRef.Disable()
	highMarkerRef.Disable()
	grabMarkerRef.Disable()

	UnregisterForAllKeys()
	; If UseJumpKey
	; 	RegisterForKey(Input.GetMappedKey("Jump")) ; jump
	; Else
	; 	RegisterForKey(ClimbKey)
	; EndIf
	
	If UseJumpKey
		SkyClimbPapyrus.RegisterClimbButton(Input.GetMappedKey("Jump"))
		RegisterForKey(Input.GetMappedKey("Jump")) ; jump
	Else
		SkyClimbPapyrus.RegisterClimbButton(ClimbKey)
		RegisterForKey(ClimbKey)
	EndIf

	SkyClimbPapyrus.RegisterClimbDelay(ButtonDelay)

	RegisterForSingleUpdate(0.05)
	
	

endFunction

; Event OnLocationChange(Location akOldLoc, Location akNewLoc)
; 	UpdateRefs()
; 	Debug.MessageBox("Location Change")
; EndEvent

bool function ParkourActive()
	;return playerRef.GetSitState() == 0 && Utility.IsInMenuMode() == false ;&& vaultMarkerRef.IsFurnitureInUse() == false && medMarkerRef.IsFurnitureInUse() == false && highMarkerRef.IsFurnitureInUse() == false
	return playerRef.GetSleepState() == 0 && playerRef.GetSitState() == 0 && Utility.IsInMenuMode() == false && playerRef.IsWeaponDrawn() == false
	
endFunction

function UpdateRefs()
	;Debug.Notification("Updating Refs")
	;bool updateLinks = false
	; Ledge Grab
	if(grabMarkerRef == none)
		grabMarkerRef = playerRef.PlaceAtMe(grabMarkerProp, 1, true, false)
		;updateLinks = true
	endIf

	if(grabActivatorRef == none)
		grabActivatorRef = playerRef.PlaceAtMe(grabActivatorProp, 1, true, false)
		;updateLinks = true
	endIf

	if(vaultActivatorRef == none)
		vaultActivatorRef = playerRef.PlaceAtMe(vaultActivatorProp, 1, true, false)
		;updateLinks = true
	endIf
	
	if(vaultMarkerRef == none)
		vaultMarkerRef = playerRef.PlaceAtMe(vaultMarkerProp, 1, true, false)
		;updateLinks = true
	endif

	if(medActivatorRef == none)
		medActivatorRef = playerRef.PlaceAtMe(medActivatorProp, 1, true, false)
		;updateLinks = true
	endIf
	
	if(medMarkerRef == none)
		medMarkerRef = playerRef.PlaceAtMe(medMarkerProp, 1, true, false)
		;updateLinks = true
	endif
	
	if(highActivatorRef == none)
		highActivatorRef = playerRef.PlaceAtMe(highActivatorProp, 1, true, false)
		;updateLinks = true
	endIf
	
	if(highMarkerRef == none)
		highMarkerRef = playerRef.PlaceAtMe(highMarkerProp, 1, true, false)
		;updateLinks = true
	endif
	
	if(indicatorRef == none)
		indicatorRef = playerRef.PlaceAtMe(indicatorObject, 1, true, false)
	endif

	; if forceUpdateLinks ; || updateLinks
		PO3_SKSEFunctions.SetLinkedRef(vaultActivatorRef, vaultMarkerRef)
		PO3_SKSEFunctions.SetLinkedRef(medActivatorRef, medMarkerRef)
		PO3_SKSEFunctions.SetLinkedRef(highActivatorRef, highMarkerRef)
		
		PO3_SKSEFunctions.SetLinkedRef(grabActivatorRef, grabMarkerRef)
	;endIf

	

endFunction

State NewGame
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


		UpdateRefs()
		; couldParkourLastFrame = canParkour

		; if climbStarted == false && ParkourActive()
		CheckStates()

	EndEvent
endstate

State AfterFirstLoad
	; Event OnBeginState()
	; 	Debug.MessageBox("Changing to optimized mode")
	; endevent
	
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
		; Since we're in the open world the moment game loads, I only do polling in maintenance and not every update unlike new game, ffs Creation kit
		
		; UpdateRefs()
		; couldParkourLastFrame = canParkour
	
		; if climbStarted == false && ParkourActive()
		CheckStates()
	
	EndEvent
endstate
; Event OnKeyUp(Int KeyCode, Float HoldTime)
; 	holdingKey = false
; EndEvent

Event OnKeyDown(Int KeyCode)
	;holdingKey = true
	UnregisterForUpdate()
	CheckStates()
EndEvent

function CheckStates()
	if !climbStarted
		holdingKey = SkyClimbPapyrus.IsClimbKeyDown()
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
				;playerRef.SetAnimationVariableBool("IsInFurniture", false) ; Clear the furniture state
				;SkyClimbPapyrus.EndAnimationEarly(playerRef)
			endif
			
			;keep em disabled
			; vaultMarkerRef.Disable()
			; medMarkerRef.Disable()
			; highMarkerRef.Disable()
			; grabMarkerRef.Disable()
			if couldParkourLastFrame == false
				vaultMarkerRef.Disable()
				medMarkerRef.Disable()
				highMarkerRef.Disable()
				grabMarkerRef.Disable()
			endif
		endif
	 endif

	; if UseJumpKey
	;     holdingKey = Input.IsKeyPressed(Input.GetMappedKey("Jump"))
	; else
	;     holdingKey = Input.IsKeyPressed(ClimbKey)
	; endif

	if holdingKey && canParkour ;&& ParkourActive()
		KeepClimbing()
		couldParkourLastFrame = true
	else
		couldParkourLastFrame = false
	endif
	RegisterForSingleUpdate(0.05)
EndFunction

function KeepClimbing()
	; if climbStarted == false && canParkour && parkourType >= 0 && ParkourActive()
	if ConsumeStamina && playerRef.GetActorValue("stamina") < StaminaDamage
		return
	endif
	
	if climbStarted == false
		;Actor playerRef = playerRef

		;playerRef.SetAnimationVariableBool("bInJumpState", false)
	
		climbStarted = true
		; holdingKey = false

		if parkourType == 5
			grabMarkerRef.Enable()
			Utility.Wait(0.01)
			grabActivatorRef.Activate(playerRef)
			Utility.Wait(0.05)
			;grabMarkerRef.Disable()
			;climbStarted = false
		elseif parkourType == 1
			medMarkerRef.Enable()
			Utility.Wait(0.01)
			medActivatorRef.Activate(playerRef)
			Utility.Wait(0.05)
			;medMarkerRef.Disable()
			;climbStarted = false
		
		elseif parkourType == 2
			highMarkerRef.Enable()
			Utility.Wait(0.01)
			highActivatorRef.Activate(playerRef)
			Utility.Wait(0.05)
			;highMarkerRef.Disable()
		elseif parkourType == 3
			vaultMarkerRef.Enable()
			Utility.Wait(0.01)
			vaultActivatorRef.Activate(playerRef)
			Utility.Wait(1.1)
			vaultMarkerRef.Disable()

			SkyClimbPapyrus.EndAnimationEarly(playerRef)
			;climbStarted = false

		endif
		
		while(playerRef.GetSitState() != 0)
		endwhile
		if ConsumeStamina
			DamagePlayerStamina.Cast(playerRef, playerRef)
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
Bool Property ConsumeStamina auto
Int Property ClimbKey Auto

float Property StaminaDamage auto
float Property ButtonDelay auto

Spell Property DamagePlayerStamina auto