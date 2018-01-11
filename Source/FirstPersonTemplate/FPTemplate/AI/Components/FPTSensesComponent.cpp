// Fill out your copyright notice in the Description page of Project Settings.

#include "FirstPersonTemplate.h"
#include "FPTSensesComponent.h"


// Sets default values for this component's properties
UFPTSensesComponent::UFPTSensesComponent()
{
	DetectableActors.Add(APawn::StaticClass());

	bCanSee = true;
	MaxSightDistance = 2800;
	PeripheralVision = 80;
	
	
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UFPTSensesComponent::BeginPlay()
{
	Super::BeginPlay();
	// ...

	
	
}


// Called every frame
void UFPTSensesComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	//Call Parent's Tick() function
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	//Run through our visual tick to determine if we saw anything this frame
	if (bCanSee)
		VisualTick(DeltaTime);

}


/* VisualTick() - Runs through all Actors of Types defined in DetectableActors in the game, and determines if we can see them
* 
*
*/
void UFPTSensesComponent::VisualTick(float DeltaTime)
{
	for (TActorIterator<AActor> ItrActor(GetWorld()); ItrActor; ++ItrActor) //Loop through our actors in the world
	{
		if (IsValidDetectableClass(ItrActor->GetClass())) //Finds only the classes we want to see per DetectableActors array
		{				
			if (CanSee(*ItrActor))// We can see them
			{
				if (!IsActorInSight(*ItrActor)) //If we haven't spotted them yet
					AddToVisibleList(*ItrActor); //Add them to the list of things we can see
			}
			else if(IsActorInSight(*ItrActor)) //We can't see them, but have they already been spotted previosly?
			{
				//If we've already spotted them, but can't see them now we've lost sight of them and need to update our information
				RemoveFromVisibleList(*ItrActor);
			}
		}
	}		
}



//==================================
//==============VISION==============
//==================================


/* IsValidDetectableClass()
*
*
*/

bool UFPTSensesComponent::IsValidDetectableClass(TSubclassOf<AActor> ActorClass)
{	
	for (uint8 i = 0; i < DetectableActors.Num(); i++)
	{
		if (ActorClass->IsChildOf(DetectableActors[i]))
			return true;
	}

	return false;
}
/* CanSee() - Checks against the specified actor, and goes through a series of checks to determine if we have a proper sight of this actor
* 
*
*/
bool UFPTSensesComponent::CanSee(AActor* ActorToCheck)
{	
	//If we have nothing to check against, or we're detecting ourselves, cancel out
	if(!ActorToCheck || !GetOwner() || ActorToCheck == GetOwner())
		return false;

	//Check to see if they're even close enough to be seen
	if (FVector::Dist(GetOwner()->GetActorLocation(), ActorToCheck->GetActorLocation()) > ReturnVisualRange()) //Target is too far away from this enemy to be seen
		return false;
	
	//Check to see if we're infront
	//@TODO: Check vision based off of head location/rotation
	FVector EnemyDifferenceVector = ActorToCheck->GetActorLocation() - GetOwner()->GetActorLocation(); //Gets the vector between this enemy and their target
	EnemyDifferenceVector.Normalize(); //Normalize the vector to turn it into a direction
	float Angle = FVector::DotProduct(GetOwner()->GetActorForwardVector(), EnemyDifferenceVector); //Figure out which way this pawn is facing compared to where they are
	Angle = FMath::RadiansToDegrees(acosf(Angle)); //Convert the Radian angle to Degrees
	if (Angle > PeripheralVision)
	{
		return false;
	}

	//Line-Trace to determine if anything is blocking our view
	//Setup Trace Parameters
	FHitResult HitResult; //The Trace Hit Results
	FCollisionQueryParams CollisionParams; //The Collision Check Options/Parameters

	CollisionParams.AddIgnoredActor(GetOwner()); //We don't want to block ourselves (well, our Pawn)

	 //Run the trace from our own Pawn to theirs
	if (GetWorld()->LineTraceSingleByChannel(HitResult, GetOwner()->GetActorLocation(), ActorToCheck->GetActorLocation(), ECC_Visibility, CollisionParams)) //We hit something on the way
		return false;

	return true;	
}

/* IsActorInSight() - Returns true if the ActorToCheck is apart of the ActorsInSight array
*
*
*/
bool UFPTSensesComponent::IsActorInSight(AActor* ActorToCheck)
{
	for (uint8 i = 0; i < VisibleActors.Num(); i++)
	{
		if (ActorToCheck == VisibleActors[i])
			return true;
	}

	return false;
}

void UFPTSensesComponent::AddToVisibleList(AActor* NewActor)
{
	if (!NewActor)
		return;

	VisibleActors.Add(NewActor);
}

void UFPTSensesComponent::RemoveFromVisibleList(AActor* RemovableActor)
{
	if (!RemovableActor || !VisibleActors.Contains(RemovableActor))
		return;

	VisibleActors.Remove(RemovableActor);
}

/* ReturnVisualRange() - 
*
*
*/
int32 UFPTSensesComponent::ReturnVisualRange()
{
	return MaxSightDistance;
}
