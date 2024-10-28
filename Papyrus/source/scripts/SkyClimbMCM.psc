Scriptname SkyClimbMCM extends Ski_ConfigBase

int climbKeyOption
int useJumpKeyOption
int enableLedgesOption
int enableVaultingOption


Event OnPageReset(string page)


	SetCursorFillMode(TOP_TO_BOTTOM)
	SetCursorPosition(0)
	
	useJumpKeyOption = AddToggleOption("Use Jump Key", sc.UseJumpKey)
	
	int flags
	if (sc.UseJumpKey)
		flags = OPTION_FLAG_DISABLED
	else
		flags = OPTION_FLAG_NONE
	endIf
	climbKeyOption = AddKeyMapOption("Climb Key", sc.ClimbKey, flags)

	enableLedgesOption = AddToggleOption("Enable Ledges", sc.EnableLedges)
	enableVaultingOption = AddToggleOption("Enable Vaulting", sc.EnableVaulting)

EndEvent

event OnOptionKeyMapChange(int option, int keyCode, string conflictControl, string conflictName)
	if (option == climbKeyOption && sc.UseJumpKey == false)
		bool continue = true
		if (conflictControl != "")
			string msg
			if (conflictName != "")
				msg = "This key is already mapped to:\n\"" + conflictControl + "\"\n(" + conflictName + ")\n\nAre you sure you want to continue?"
			else
				msg = "This key is already mapped to:\n\"" + conflictControl + "\"\n\nAre you sure you want to continue?"
			endIf

			continue = ShowMessage(msg, true, "$Yes", "$No")
		endIf

		if (continue)
			sc.ClimbKey = keyCode
			;sc.UnregisterForAllKeys()
			;sc.RegisterForKey(sc.ClimbKey)
			SkyClimbPapyrus.RegisterClimbButton(sc.ClimbKey)
			SetKeyMapOptionValue(climbKeyOption, sc.ClimbKey)
		endIf

	endIf
endEvent

event OnOptionSelect(int option)
	if (option == useJumpKeyOption)
		sc.UseJumpKey = !sc.UseJumpKey
		
		;sc.UnregisterForAllKeys()
		
		if sc.UseJumpKey == false
			SkyClimbPapyrus.ToggleJumping(true)
			;sc.RegisterForKey(sc.ClimbKey)
			SkyClimbPapyrus.RegisterClimbButton(sc.ClimbKey)
		else
			; sc.RegisterForKey(Input.GetMappedKey("Jump"))
			SkyClimbPapyrus.RegisterClimbButton(Input.GetMappedKey("Jump"))
		endif
		SetToggleOptionValue(option, sc.UseJumpKey)
		
		ForcePageReset()
	elseif (option == enableLedgesOption)
		sc.EnableLedges = !sc.EnableLedges
		SetToggleOptionValue(option, sc.EnableLedges)
	elseif (option == enableVaultingOption)
		sc.EnableVaulting = !sc.EnableVaulting
		SetToggleOptionValue(option, sc.EnableVaulting)
	endIf
endEvent

SkyClimbQuestScript Property sc Auto