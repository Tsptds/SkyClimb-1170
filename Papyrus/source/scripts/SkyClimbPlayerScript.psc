Scriptname SkyClimbPlayerScript extends ReferenceAlias  


Event OnPlayerLoadGame()
	if(isNewGame)
		(GetOwningQuest() as SkyClimbQuestScript).GoToState("AfterFirstLoad")
		isNewGame = false
		;Debug.MessageBox("Switching to AfterFirstLoad")
	endif
	(GetOwningQuest() as SkyClimbQuestScript).Maintenance()
EndEvent

Event OnInit()
	if(isNewGame)
		(GetOwningQuest() as SkyClimbQuestScript).GoToState("NewGame")
		;Debug.MessageBox("New Game started")
	endif
	(GetOwningQuest() as SkyClimbQuestScript).Maintenance()
endevent
bool property isNewGame auto