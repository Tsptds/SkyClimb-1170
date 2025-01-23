Scriptname SkyParkourQuestScript extends Quest  

bool property climbStarted auto

bool property holdingKey auto

int property parkourType auto

Actor property playerRef auto

Event OnInit()
	Maintenance()
EndEvent

function Maintenance()
	;Debug.MessageBox("Maintenance")

	climbStarted = false
	
	parkourType = -1

	indicatorRef.Disable()

	UnregisterForAllKeys()
		
	If UseJumpKey
		SkyParkourPapyrus.RegisterClimbButton(Input.GetMappedKey("Jump"))
		RegisterForKey(Input.GetMappedKey("Jump")) ; jump

	Else
		SkyParkourPapyrus.RegisterClimbButton(ClimbKey)
		RegisterForKey(ClimbKey)

	EndIf

	SkyParkourPapyrus.RegisterClimbDelay(ButtonDelay)

	SkyParkourPapyrus.RegisterReferences(vaultMarkerRef, lowMarkerRef, medMarkerRef, highMarkerRef, indicatorRef)

	RegisterForSingleUpdate(0.25)
	
endFunction

	
Event OnUpdate()

	holdingKey = SkyParkourPapyrus.IsClimbKeyDown()
	CheckStates()
	
EndEvent

Event OnKeyDown(Int KeyCode)
	UnregisterForUpdate()
	holdingKey = SkyParkourPapyrus.IsClimbKeyDown()
	CheckStates()

EndEvent

function CheckStates()
	if SkyParkourPapyrus.IsParkourActive() && !climbStarted
		parkourType = SkyParkourPapyrus.UpdateParkourPoint(UseJumpKey, EnableVaulting, EnableLedges)
	
		if parkourType != -1
			
			indicatorRef.Enable()
			
			if holdingKey
				KeepClimbing()
			endif
	
		else
			indicatorRef.Disable()

		endif

	else
		indicatorRef.Disable()

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
			lowActivatorRef.Activate(playerRef)

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

ObjectReference Property highMarkerRef Auto
ObjectReference Property medMarkerRef Auto
ObjectReference Property vaultMarkerRef Auto 
ObjectReference Property highActivatorRef Auto 
ObjectReference Property medActivatorRef Auto 
ObjectReference Property vaultActivatorRef Auto 

ObjectReference Property lowMarkerRef auto 
ObjectReference Property lowActivatorRef auto 

;Activator Property highActivatorProp Auto
;Activator Property medActivatorProp Auto
;Activator Property vaultActivatorProp Auto
;Furniture Property highMarkerProp Auto
;Furniture Property medMarkerProp Auto
;Furniture Property vaultMarkerProp Auto

;Furniture Property lowMarkerProp auto
;Activator Property lowActivatorProp auto

ObjectReference Property indicatorRef Auto
;MiscObject Property indicatorObject Auto

Bool Property EnableLedges Auto
Bool Property EnableVaulting Auto
Bool Property UseJumpKey Auto
Bool Property ConsumeStamina auto
Int Property ClimbKey Auto

float Property StaminaDamage auto
float Property ButtonDelay auto
;Bool Property wasSneaking auto

Spell Property DamagePlayerStamina auto