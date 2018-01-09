// Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.

#include "FirstPersonTemplate.h"
#include "FPTDamageInterface.h"

//////////////////////////////////////////////////////////////////////////
// ToStringInterface

UFPTDamageInterface::UFPTDamageInterface(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}


void IFPTDamageInterface::ReceiveDamage_Implementation(uint8 DamageAmount,FHitResult HitInfo,AActor* DamageCausingActor,AActor* DamageOwner)
{
	UE_LOG(LogFPT, All, TEXT("!!!!!WARNING!!!!! - CLASS has not overridden ReceiveDamage()"));
}