Scriptname SkyClimbMCM extends Ski_ConfigBase

int climbKeyOption
int useJumpKeyOption
int enableLedgesOption
int enableVaultingOption
int useStaminaOption
int staminaSlider
int climbDelaySlider

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
	
	;Stamina consumption
	useStaminaOption = AddToggleOption("Enable Stamina Consumption", sc.ConsumeStamina)
	staminaSlider = AddSliderOption("Stamina Damage Value", sc.StaminaDamage)

	;Climb Delay
	climbDelaySlider = AddSliderOption("Button Delay Seconds", sc.ButtonDelay, "{1}s")
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
			sc.UnregisterForAllKeys()
			sc.RegisterForKey(sc.ClimbKey)
			SkyClimbPapyrus.RegisterClimbButton(sc.ClimbKey)
			SetKeyMapOptionValue(climbKeyOption, sc.ClimbKey)
		endIf

	endIf
endEvent

event OnOptionSelect(int option)
	if (option == useJumpKeyOption)
		sc.UseJumpKey = !sc.UseJumpKey
		sc.UpdateRefs()
		sc.UnregisterForAllKeys()
		
		if sc.UseJumpKey == false
			SkyClimbPapyrus.ToggleJumping(true)
			sc.RegisterForKey(sc.ClimbKey)
			SkyClimbPapyrus.RegisterClimbButton(sc.ClimbKey)
		else
			sc.RegisterForKey(Input.GetMappedKey("Jump"))
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
	elseif (option == useStaminaOption)
		sc.ConsumeStamina = !sc.ConsumeStamina
		SetToggleOptionValue(option, sc.ConsumeStamina)
	endIf
endEvent

event OnOptionSliderOpen(int a_option)
	{Called when the user selects a slider option}

	if (a_option == staminaSlider)
		SetSliderDialogStartValue(sc.staminaDamage)
		SetSliderDialogDefaultValue(25)
		SetSliderDialogRange(0, 100)
		SetSliderDialogInterval(1)
	endIf

	if (a_option == climbDelaySlider)
		SetSliderDialogStartValue(sc.ButtonDelay)
		SetSliderDialogDefaultValue(0.0)
		SetSliderDialogRange(0.0, 0.5)
		SetSliderDialogInterval(0.1)
	endif
endEvent

event OnOptionSliderAccept(int a_option, float a_value)
	{Called when the user accepts a new slider value}
		
	if (a_option == staminaSlider)
		sc.StaminaDamage = a_value
		SetSliderOptionValue(a_option, a_value)
	endIf

	if (a_option == climbDelaySlider)
		sc.ButtonDelay = a_value
		SetSliderOptionValue(a_option, a_value, "{1}s")
		SkyClimbPapyrus.RegisterClimbDelay(a_value)
	endif
endEvent

SkyClimbQuestScript Property sc Auto