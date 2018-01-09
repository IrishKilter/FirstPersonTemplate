// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "../FPTCharacter.h"
#include "../Players/FPTPlayerPawn.h"
#include "FPTInventoryManager.h"
#include "FPTInventoryItem.generated.h"

UCLASS(abstract)
class FIRSTPERSONTEMPLATE_API AFPTInventoryItem : public AActor
{
	GENERATED_BODY()


	UPROPERTY(Replicated)
	class AFPTInventoryManager* InvManager;



	//=================================================================================================
	//============================================FUNCTIONS============================================
	//=================================================================================================

	//======================================
	//============INITIALIZATION============
	//======================================
public:	
	// Sets default values for this actor's properties
	AFPTInventoryItem();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
public:
	UFUNCTION()
	void RegisterItem(class AFPTInventoryManager* OwningInvManager);


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;	
	


	UFUNCTION()
	class AFPTCharacter* ReturnOwningPawn();


	//=====================
	//======ATTACHMENT=====
	//=====================
public:
	UFUNCTION(NetMulticast,Reliable) //Event called to let this item know it's being equipped
	virtual void OnEquipped();
	UFUNCTION(NetMulticast,Reliable) //Event called to let this item konw it's being unequipped
	virtual void OnUnequipped();

};
