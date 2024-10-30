Scriptname SkyClimbPlayerScript extends ReferenceAlias  


Event OnPlayerLoadGame()
	(GetOwningQuest() as SkyClimbQuestScript).Maintenance()
	if(isNewGame)
		(GetOwningQuest() as SkyClimbQuestScript).GoToState("AfterFirstLoad")
		isNewGame = false
	endif
EndEvent
Event OnInit()
	(GetOwningQuest() as SkyClimbQuestScript).Maintenance()
endevent
bool property isNewGame auto