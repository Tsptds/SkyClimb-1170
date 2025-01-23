Scriptname SkyParkourDamageStaminaScript extends activemagiceffect  

SkyParkourQuestScript Property sc  Auto  

Actor Property Player  Auto 
float dmg

Event OnEffectStart(Actor target, Actor caster)
	dmg=sc.StaminaDamage
	Player.DamageActorValue("stamina", dmg)
endEvent