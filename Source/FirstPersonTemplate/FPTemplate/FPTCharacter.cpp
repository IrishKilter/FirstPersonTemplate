// Fill out your copyright notice in the Description page of Project Settings.

#include "FirstPersonTemplate.h"
#include "FPTCharacter.h"


// Sets default values
AFPTCharacter::AFPTCharacter(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	Health = 100.0f;
	
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AFPTCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AFPTCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}



void AFPTCharacter::ReceiveDamage_Implementation(uint8 DamageAmount, FHitResult HitInfo, AActor* DamageCausingActor, AActor* DamageOwner)
{
	Health -= DamageAmount;

	if (Health <= 0)
		Die(DamageCausingActor, DamageOwner);

}



void AFPTCharacter::Die(AActor* DeathCausingActor, AActor* DeathInstigator)
{
	Destroy();
}