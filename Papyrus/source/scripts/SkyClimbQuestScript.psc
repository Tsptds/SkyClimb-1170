Scriptname SkyClimbQuestScript extends Quest  

bool property climbStarted auto

bool property holdingKey auto

int property parkourType auto

Actor property playerRef auto

Event OnInit()

	;string hello = SkyClimbPapyrus.SayHello()
	
	;Debug.MessageBox(hello)
	
	;Maintenance()
EndEvent

function Maintenance()
	;Debug.MessageBox("Maintenance")
	UpdateRefs()

	climbStarted = false
	
	parkourType = -1

	indicatorRef.Disable()

	vaultMarkerRef.Enable()
	medMarkerRef.Enable()
	highMarkerRef.Enable()
	grabMarkerRef.Enable()

	UnregisterForAllKeys()
		
	If UseJumpKey
		SkyClimbPapyrus.RegisterClimbButton(Input.GetMappedKey("Jump"))
		RegisterForKey(Input.GetMappedKey("Jump")) ; jump

	Else
		SkyClimbPapyrus.RegisterClimbButton(ClimbKey)
		RegisterForKey(ClimbKey)

	EndIf

	SkyClimbPapyrus.RegisterClimbDelay(ButtonDelay)

	RegisterForSingleUpdate(0.25)
	
endFunction

function UpdateRefs()
	
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

	PO3_SKSEFunctions.SetLinkedRef(vaultActivatorRef, vaultMarkerRef)
	PO3_SKSEFunctions.SetLinkedRef(medActivatorRef, medMarkerRef)
	PO3_SKSEFunctions.SetLinkedRef(highActivatorRef, highMarkerRef)
	PO3_SKSEFunctions.SetLinkedRef(grabActivatorRef, grabMarkerRef)

endFunction

State NewGame
	Event OnUpdate()

		if !SkyClimbPapyrus.IsParkourActive()
			indicatorRef.Disable()
			RegisterForSingleUpdate(0.10)
			return

		endif


		UpdateRefs()
		
		holdingKey = SkyClimbPapyrus.IsClimbKeyDown()
		CheckStates()

	EndEvent
endstate

State AfterFirstLoad
	; Event OnBeginState()
	; 	Debug.MessageBox("Changing to optimized mode")
	; endevent
	
	Event OnUpdate()

		if !SkyClimbPapyrus.IsParkourActive()
			indicatorRef.Disable()
			RegisterForSingleUpdate(0.10)
			return

		endif

		; Since we're in the open world the moment game loads, I only do polling in maintenance and not every update unlike new game, ffs Creation kit, OnInit fires before game world loads in
		; UpdateRefs()	

		holdingKey = SkyClimbPapyrus.IsClimbKeyDown()
		CheckStates()
	
	EndEvent
endstate

Event OnKeyDown(Int KeyCode)
	UnregisterForUpdate()
	holdingKey = true
	CheckStates()

EndEvent

function CheckStates()
	if !climbStarted
		parkourType = SkyClimbPapyrus.UpdateParkourPoint(vaultMarkerRef, medMarkerRef, highMarkerRef, indicatorRef, UseJumpKey, EnableVaulting, EnableLedges, grabMarkerRef)
	
		if parkourType != -1 ;&& SkyClimbPapyrus.IsParkourActive()
			
			indicatorRef.Enable()
			
			if holdingKey
				KeepClimbing()
			endif

		else
			indicatorRef.Disable()

		endif

	 endif

	RegisterForSingleUpdate(0.10)
EndFunction

function KeepClimbing()
	;if ConsumeStamina && playerRef.GetActorValue("stamina") < StaminaDamage
	;	return
	;endif
	
	if climbStarted == false
			
		climbStarted = true

		if parkourType == 5
			grabActivatorRef.Activate(playerRef)

		elseif parkourType == 1
			medActivatorRef.Activate(playerRef)
					
		elseif parkourType == 2
			highActivatorRef.Activate(playerRef)
			
		elseif parkourType == 3
			vaultActivatorRef.Activate(playerRef)

		endif

		if ConsumeStamina
			DamagePlayerStamina.Cast(playerRef, playerRef)
		endif

		Utility.Wait(0.05) ; Otherwise skips while loop cause activation may be delayed

		while(playerRef.GetSitState() != 0) ; no need to register for time being
		endwhile
		
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
;Bool Property wasSneaking auto

Spell Property DamagePlayerStamina auto