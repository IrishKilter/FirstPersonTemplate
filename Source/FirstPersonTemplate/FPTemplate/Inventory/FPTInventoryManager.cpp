// Fill out your copyright notice in the Description page of Project Settings.

#include "FirstPersonTemplate.h"
#include "FPTInventoryItem.h"
#include "FPTInventoryManager.h"


// Sets default values
AFPTInventoryManager::AFPTInventoryManager()
{
 	//Replication
	bReplicates = true; //Allows replication
	bOnlyRelevantToOwner = true; //Keeps it strictly replicated to the owner
	bAlwaysRelevant = true; //Sends replication updates regardless of distance

	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AFPTInventoryManager::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AFPTInventoryManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


void AFPTInventoryManager::InitInvManager(AActor* NewOwner)
{
	if (!NewOwner)
		return;

	SetOwner(NewOwner);
}



//======================================================
//=================INVENTORY MANAGEMENT=================
//======================================================
void AFPTInventoryManager::AddItem(AFPTInventoryItem* Item)
{
	UE_LOG(LogFPTInventory, All, TEXT("%s::AFPTInventoryManager::AddItem()"),*GetName());

	if (!Item)
		return;

	
	RegisterItem(Item);

}

void AFPTInventoryManager::RemoveItem(AFPTInventoryItem* Item)
{
	if (!Item)
		return;

}

void AFPTInventoryManager::RegisterItem(AFPTInventoryItem* Item)
{
	UE_LOG(LogFPTInventory, All, TEXT("%s::AFPTInventoryManager::RegisterItem()"),*GetName());

	if (!Item)
		return;

	Item->RegisterItem(this);
}

void AFPTInventoryManager::UnregisterItem(AFPTInventoryItem* Item)
{
	if(!Item)
		return;
}

/* IsItemRegistered() - Checks to see if the item in question is registered to this particular Inventory Manager
*
*
*/
bool AFPTInventoryManager::IsItemRegistered(AFPTInventoryItem* Item)
{
	if (!Item) //Item doesn't exist, so it's not registered
		return false;

	if (Item->GetOwner() == this)
		return true;

	return false;
}