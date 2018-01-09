// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "FPTInventoryManager.generated.h"

UCLASS()
class FIRSTPERSONTEMPLATE_API AFPTInventoryManager : public AActor
{
	GENERATED_BODY()
	

	//=========================================================================================================
	//================================================FUNCTIONS================================================
	//=========================================================================================================

	//====================
	//===Initializaiton===
	//====================
public:
	// Sets default values for this actor's properties
	AFPTInventoryManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void InitInvManager(AActor* NewOwner);



	//==========================
	//===INVENTORY MANAGEMENT===
	//==========================
public:
	UFUNCTION(BlueprintCallable, Category = "Inventory") //Adds the item into our inventory
	void AddItem(class AFPTInventoryItem* Item);
	UFUNCTION(BlueprintCallable, Category = "Inventory") //Removes the item from our inventory
	void RemoveItem(class AFPTInventoryItem* Item);
	UFUNCTION() //Registers the item to this particular inventory manager
	void RegisterItem(class AFPTInventoryItem* Item);
	UFUNCTION()
	void UnregisterItem(class AFPTInventoryItem* Item);
public:
	UFUNCTION() //Checks to see if the item in question is apart of this Inventory Manager
	bool IsItemRegistered(class AFPTInventoryItem* Item);

};
