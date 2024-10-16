Scriptname SkyClimbQuestScript extends Quest  

bool canParkour = false
;bool couldParkourLastFrame = false

bool climbStarted = false

bool holdingKey = false

int parkourType = -1

float lastParkourPosX
float lastParkourPosY
float lastParkourPosZ

function OnInit()

	;string hello = SkyClimbPapyrus.SayHello()
	
	;Debug.MessageBox(hello)
	
	Maintenance()
	
endFunction

function Maintenance()

	UpdateRefs(true)

	lastParkourPosX = 0
	lastParkourPosY = 0
	lastParkourPosZ = 0
	
	;couldParkourLastFrame = false
	canParkour = false
	climbStarted = false
	
	parkourType = -1
	indicatorRef.Disable()
	
	
	UnregisterForAllKeys()
	If UseJumpKey
		RegisterForKey(Input.GetMappedKey("Jump")) ; jump
	Else
		RegisterForKey(ClimbKey)
	EndIf
	
	RegisterForSingleUpdate(0.05)
	
	

endFunction

bool function ParkourActive()

	return Game.GetPlayer().GetSitState() == 0 && Utility.IsInMenuMode() == false ;&& vaultMarkerRef.IsFurnitureInUse() == false && medMarkerRef.IsFurnitureInUse() == false && highMarkerRef.IsFurnitureInUse() == false

endFunction

function UpdateRefs(bool forceUpdateLinks)

	bool updateLinks = false
	
	
	if(vaultActivatorRef == none)
		vaultActivatorRef = Game.GetPlayer().PlaceAtMe(vaultActivatorProp, 1, true, false)
		updateLinks = true
	endIf
	
	if(vaultMarkerRef == none)
		vaultMarkerRef = Game.GetPlayer().PlaceAtMe(vaultMarkerProp, 1, true, false)
		updateLinks = true
	endif

	if(medActivatorRef == none)
		medActivatorRef = Game.GetPlayer().PlaceAtMe(medActivatorProp, 1, true, false)
		updateLinks = true
	endIf
	
	if(medMarkerRef == none)
		medMarkerRef = Game.GetPlayer().PlaceAtMe(medMarkerProp, 1, true, false)
		updateLinks = true
	endif
	
	if(highActivatorRef == none)
		highActivatorRef = Game.GetPlayer().PlaceAtMe(highActivatorProp, 1, true, false)
		updateLinks = true
	endIf
	
	if(highMarkerRef == none)
		highMarkerRef = Game.GetPlayer().PlaceAtMe(highMarkerProp, 1, true, false)
		updateLinks = true
	endif
	
	if updateLinks || forceUpdateLinks
		PO3_SKSEFunctions.SetLinkedRef(vaultActivatorRef, vaultMarkerRef)
		PO3_SKSEFunctions.SetLinkedRef(medActivatorRef, medMarkerRef)
		PO3_SKSEFunctions.SetLinkedRef(highActivatorRef, highMarkerRef)
	endIf

	if(indicatorRef == none)
		indicatorRef = Game.GetPlayer().PlaceAtMe(indicatorObject, 1, true, false)
	endif

endFunction

Event OnUpdate()

	if !ParkourActive()
		if canParkour
			canParkour = false
			indicatorRef.Disable()
		endif
		RegisterForSingleUpdate(0.03)
		return
	endif


	UpdateRefs(true)
	; couldParkourLastFrame = canParkour

	; if climbStarted == false && ParkourActive()
	if !climbStarted
	
		parkourType = SkyClimbPapyrus.UpdateParkourPoint(vaultMarkerRef, medMarkerRef, highMarkerRef, indicatorRef, UseJumpKey, EnableVaulting, EnableLedges)
	
		if parkourType >= 0
			
			if canParkour == false
				canParkour = true
				indicatorRef.Enable()
			endif

		else
			if canParkour == true
				canParkour = false
				indicatorRef.Disable()
					
			endif
			
			;keep em disabled
			vaultMarkerRef.Disable()
			medMarkerRef.Disable()
			highMarkerRef.Disable()
		endif
	 endif

	
	if holdingKey ;&& ParkourActive()
		KeepClimbing()
	endif
	RegisterForSingleUpdate(0.03)

EndEvent

Event OnKeyUp(Int KeyCode, Float HoldTime)
	holdingKey = false
EndEvent

Event OnKeyDown(Int KeyCode)
	holdingKey = true
EndEvent

function KeepClimbing()
	; if climbStarted == false && couldParkourLastFrame && canParkour && parkourType >= 0 && ParkourActive()		;Default couldParkourLastFrame
	if climbStarted == false && canParkour
		Actor playerRef = Game.GetPlayer()

		;playerRef.SetAnimationVariableBool("bInJumpState", false)
	
		climbStarted = true

	
		if parkourType == 1
			medMarkerRef.Enable()
			Utility.Wait(0.01)
			medActivatorRef.Activate(playerRef)
			;Utility.Wait(0.05)

			Utility.Wait(2.2)
			medMarkerRef.Disable()
			SkyClimbPapyrus.EndAnimationEarly(playerRef)

			climbStarted = false
		
		elseif parkourType == 2
			highMarkerRef.Enable()
			Utility.Wait(0.01)
			highActivatorRef.Activate(playerRef)
			;Utility.Wait(0.05)
			
			Utility.Wait(2.7)
			highMarkerRef.Disable()
			SkyClimbPapyrus.EndAnimationEarly(playerRef)

			climbStarted = false
		
		; elseif parkourType == 3
		; 	vaultMarkerRef.Enable()
		; 	Utility.Wait(0.01)
		; 	vaultActivatorRef.Activate(playerRef)
		; 	Utility.Wait(0.05)
		; 	climbStarted = false
		
		; elseif parkourType == 4
		; 	vaultMarkerRef.Enable()
		; 	Utility.Wait(0.01)
		; 	vaultActivatorRef.Activate(playerRef)
		; 	Utility.Wait(1.1)
		; 	vaultMarkerRef.Disable()
		
		; 	SkyClimbPapyrus.EndAnimationEarly(playerRef)
		; 	climbStarted = false
		
		; Always cut off landing animation for smoother vaults
		elseif parkourType == 3
			vaultMarkerRef.Enable()
			Utility.Wait(0.01)
			vaultActivatorRef.Activate(playerRef)
			Utility.Wait(1.1)
			vaultMarkerRef.Disable()

			SkyClimbPapyrus.EndAnimationEarly(playerRef)
			climbStarted = false
		endif
		;UpdateRefs(true)
	endIf

endfunction

ObjectReference Property highMarkerRef Auto Hidden
ObjectReference Property medMarkerRef Auto Hidden
ObjectReference Property vaultMarkerRef Auto Hidden
ObjectReference Property highActivatorRef Auto Hidden
ObjectReference Property medActivatorRef Auto Hidden
ObjectReference Property vaultActivatorRef Auto Hidden

Activator Property highActivatorProp Auto
Activator Property medActivatorProp Auto
Activator Property vaultActivatorProp Auto
Furniture Property highMarkerProp Auto
Furniture Property medMarkerProp Auto
Furniture Property vaultMarkerProp Auto

ObjectReference Property indicatorRef Auto Hidden
MiscObject Property indicatorObject Auto

Bool Property EnableLedges Auto
Bool Property EnableVaulting Auto
Bool Property UseJumpKey Auto
Int Property ClimbKey Auto
