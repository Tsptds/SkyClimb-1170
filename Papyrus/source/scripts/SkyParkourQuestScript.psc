Scriptname SkyParkourQuestScript extends Quest  

int property parkourType auto

Event OnInit()
EndEvent

function Maintenance()
	;Debug.MessageBox("Maintenance")

	UnregisterForAllKeys()
		
	If UseJumpKey
		SkyParkourPapyrus.RegisterClimbButton(Input.GetMappedKey("Jump"))

	Else
		SkyParkourPapyrus.RegisterClimbButton(ClimbKey)

	EndIf

	SkyParkourPapyrus.RegisterClimbDelay(ButtonDelay)

	SkyParkourPapyrus.RegisterReferences(vaultMarkerRef, lowMarkerRef, medMarkerRef, highMarkerRef, vaultActivatorRef, lowActivatorRef, medActivatorRef, highActivatorRef, indicatorRef)

	SkyParkourPapyrus.RegisterStaminaDamage(ConsumeStamina, StaminaDamage)

	RegisterForSingleUpdate(0.25)
	
endFunction

	
Event OnUpdate()
	CheckStates()
EndEvent

function CheckStates()
	parkourType = SkyParkourPapyrus.UpdateParkourPoint(UseJumpKey, EnableVaulting, EnableLedges)
	RegisterForSingleUpdate(0.10)
EndFunction

ObjectReference Property highMarkerRef Auto
ObjectReference Property medMarkerRef Auto
ObjectReference Property vaultMarkerRef Auto 
ObjectReference Property highActivatorRef Auto 
ObjectReference Property medActivatorRef Auto 
ObjectReference Property vaultActivatorRef Auto 

ObjectReference Property lowMarkerRef auto 
ObjectReference Property lowActivatorRef auto

ObjectReference Property indicatorRef Auto

Bool Property EnableLedges Auto
Bool Property EnableVaulting Auto
Bool Property UseJumpKey Auto
Bool Property ConsumeStamina auto
Int Property ClimbKey Auto

float Property StaminaDamage auto
float Property ButtonDelay auto