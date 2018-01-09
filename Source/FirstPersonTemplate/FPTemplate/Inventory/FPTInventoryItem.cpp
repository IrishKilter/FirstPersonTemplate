// Fill out your copyright notice in the Description page of Project Settings.

#include "FirstPersonTemplate.h"
#include "FPTInventoryItem.h"

//Replications
void AFPTInventoryItem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//DOREPLIFETIME(AFPTInventoryItem, Owner);
	DOREPLIFETIME_CONDITION(AFPTInventoryItem, InvManager, COND_OwnerOnly);
}


// Sets default values
AFPTInventoryItem::AFPTInventoryItem()
{
 	
	//Replication
	bOnlyRelevantToOwner = false;
	bReplicates = true;

	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AFPTInventoryItem::BeginPlay()
{
	Super::BeginPlay();
	
}


void AFPTInventoryItem::RegisterItem(AFPTInventoryManager* OwningInvManager)
{
	UE_LOG(LogFPTInventory, All, TEXT("%s::RegisterItem()"), *GetName());

	if (!OwningInvManager)
		return;

//	InvManager = OwningInvManager;
	SetOwner(OwningInvManager);
}

// Called every frame
void AFPTInventoryItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}



AFPTCharacter* AFPTInventoryItem::ReturnOwningPawn()
{
	//Make sure we actually have an owning pawn
	if (!GetOwner() || !GetOwner()->GetOwner()) //1st Owner = InvManager, 2nd Owner (InvManger's Owner) = Pawn
		return nullptr;

	//Return Inventory Manager's Owner (Owning Pawn)
	if (Cast<AFPTCharacter>(GetOwner()->GetOwner()))
		return Cast<AFPTCharacter>(GetOwner()->GetOwner());
	else
		return nullptr;
}



//============================
//=========ATTACHMENT=========
//============================

void AFPTInventoryItem::OnEquipped_Implementation()
{

}

void AFPTInventoryItem::OnUnequipped_Implementation()
{

}