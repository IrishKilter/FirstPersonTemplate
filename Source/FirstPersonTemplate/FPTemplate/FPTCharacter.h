// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/FPTDamageInterface.h"
#include "GameFramework/Character.h"
#include "FPTCharacter.generated.h"

UCLASS()
class FIRSTPERSONTEMPLATE_API AFPTCharacter : public ACharacter, public IFPTDamageInterface
{
	GENERATED_BODY()


	//========================
	//=========HEALTH=========
	//========================
protected:
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category = "Health",meta = (UIMin = 0, ClampMin = 0))
	float Health;


	//==========================================================================================================================================================================================
	// ============================================================================ FUNCTIONS ==================================================================================================
	//==========================================================================================================================================================================================

public:
	// Sets default values for this character's properties
	AFPTCharacter(const FObjectInitializer& ObjectInitializer);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


	//=========================================
	//=============DAMAGE HANDLING=============
	//=========================================
	virtual void ReceiveDamage_Implementation(uint8 DamageAmount, FHitResult HitInfo, AActor* DamageCausingActor, AActor* DamageOwner) override;

protected:
	UFUNCTION()
	virtual void Die(AActor* DeathCausingActor,AActor* DeathInstigator);

};
