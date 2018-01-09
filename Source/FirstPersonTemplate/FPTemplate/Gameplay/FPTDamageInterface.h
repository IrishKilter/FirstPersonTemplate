#pragma once

#include "FPTDamageInterface.generated.h"

/** Class needed to support InterfaceCast<IToStringInterface>(Object) */
UINTERFACE(MinimalAPI)
class UFPTDamageInterface : public UInterface
{
	GENERATED_UINTERFACE_BODY()
};

class IFPTDamageInterface
{
	GENERATED_IINTERFACE_BODY()

		
		/* 
		* DamageAmount - How much damage was sent to us
		* DamageCausingActor - IE: The Weapon that shot us
		* Who's responsible for causing the damage (IE: The player holding the weapon)
		* 
		*/ 
		UFUNCTION(BlueprintNativeEvent,BlueprintCallable,Category = "Damage") //Receiving the Damage Information!
		void ReceiveDamage(uint8 DamageAmount,FHitResult HitInfo, AActor* DamageCausingActor, AActor* DamageOwner);

};