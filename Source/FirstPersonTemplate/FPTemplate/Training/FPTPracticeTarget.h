// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../Gameplay/FPTDamageInterface.h"
#include "FPTPracticeTarget.generated.h"

UCLASS()
class FIRSTPERSONTEMPLATE_API AFPTPracticeTarget : public AActor, public IFPTDamageInterface
{
	GENERATED_BODY()

	//====Target States===
protected:
	UPROPERTY(ReplicatedUsing=OnRep_UpdateStance) //Used to replicate the stance of this Target (Standing or Knocked Down)
	bool bKnockedDown; 

	//===Target Properties===
	UPROPERTY(EditInstanceOnly, Category = "Target Properties") //Set to false if you want to manually control the popup/knockdown via Blueprints
	bool bUseKnockDownTimer;
	UPROPERTY(EditInstanceOnly, Category = "Target Properties") //How long this target stays down before popping up (bUseKnockDownTimer MUST be true)
	float KnockDownTime;
	FTimerHandle KnockDownTimer; //The actual timer to handle the knock down and pop ups



	//===MESH===
protected:
	UPROPERTY(VisibleAnywhere, Category = "Mesh")
	UStaticMeshComponent* Mesh;
	UPROPERTY(VisibleAnywhere, Category = "Mesh")
	UArrowComponent* ForwardArrow;

	

	
public:	
	// Sets default values for this actor's properties
	AFPTPracticeTarget(const FObjectInitializer& ObjectInitializer);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//======================
	//========DAMAGE========
	//======================
	virtual void ReceiveDamage_Implementation(uint8 DamageAmount,FHitResult HitInfo,AActor* DamageCausingActor,AActor* DamageOwner) override;
	
	UFUNCTION()
	void OnRep_UpdateStance();
	UFUNCTION()
	void PopUp();
	UFUNCTION() //Knocks down the target
	void KnockDown();
};
