// Fill out your copyright notice in the Description page of Project Settings.

#include "FirstPersonTemplate.h"
#include "FPTPracticeTarget.h"


void AFPTPracticeTarget::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	//Call all of the parents replicated construction
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AFPTPracticeTarget, bKnockedDown);
	
}

// Sets default values
AFPTPracticeTarget::AFPTPracticeTarget(const FObjectInitializer& ObjectInitializer)
{
	//Target Properties
	bUseKnockDownTimer = true; //By default knock down, and pop back up
	KnockDownTime = 3.0f; //3 seconds by default

	
	//Initialize the Mesh
	Mesh = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("TargetMeshComp"));
	
	//Set the default mesh for the target
	if(Mesh) //Make sure Mesh is initialized properly
	{
		SetRootComponent(Mesh);

		static ConstructorHelpers::FObjectFinder<UStaticMesh> TempMesh(TEXT("/Game/Template/Props/Military/Mesh/ETypeTarget.ETypeTarget")); //Grab the reference

		if (TempMesh.Object)//if we properly referenced the asset
			Mesh->SetStaticMesh((UStaticMesh*)TempMesh.Object);
	}

	//Initialize the Arrow Component that tells us which way the target is facing (thus which way it'll swing back!)
	ForwardArrow = ObjectInitializer.CreateDefaultSubobject<UArrowComponent>(this, TEXT("ForwardArrowComp"));
	if (ForwardArrow)
	{
		ForwardArrow->AttachToComponent(Mesh,FAttachmentTransformRules::SnapToTargetIncludingScale);
	}

	//Replication Setup
	bReplicates = true;
	
	PrimaryActorTick.bCanEverTick = false; //No need for this to tick. Event-based only should be good

}

// Called when the game starts or when spawned
void AFPTPracticeTarget::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AFPTPracticeTarget::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


//======================================
//================DAMAGE================
//======================================
void AFPTPracticeTarget::ReceiveDamage_Implementation(uint8 DamageAmount,FHitResult HitInfo, AActor* DamageCausingActor, AActor* DamageOwner)
{
	UE_LOG(LogFPT, All, TEXT("!!!%s::ReceiveDamage(%i,%s,%s)!!!"), *GetName(), DamageAmount,*DamageCausingActor->GetName(),*DamageOwner->GetName());
	KnockDown();
}

void AFPTPracticeTarget::OnRep_UpdateStance()
{
	if (bKnockedDown)
		KnockDown();
	else
		PopUp();
}

void AFPTPracticeTarget::PopUp()
{
	SetActorRotation(FRotator(0, 0, 0));
	bKnockedDown = false;
}

void AFPTPracticeTarget::KnockDown()
{
	SetActorRotation(FRotator(90, 0, 0));
	bKnockedDown = true;

	if (bUseKnockDownTimer)
		GetWorldTimerManager().SetTimer(KnockDownTimer, this,&AFPTPracticeTarget::PopUp, KnockDownTime, false);
}

