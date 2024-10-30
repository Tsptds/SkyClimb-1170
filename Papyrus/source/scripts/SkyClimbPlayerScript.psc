Scriptname SkyClimbPlayerScript extends ReferenceAlias  


Event OnPlayerLoadGame()
	(GetOwningQuest() as SkyClimbQuestScript).Maintenance()
EndEvent