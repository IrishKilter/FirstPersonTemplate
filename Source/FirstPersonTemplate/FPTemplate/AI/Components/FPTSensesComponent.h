// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FPTSensesComponent.generated.h"


UCLASS( ClassGroup=(AI), meta=(BlueprintSpawnableComponent, DisplayName = "FPTSensesComponent"), HideCategories=(Cooking,Collision) )
class FIRSTPERSONTEMPLATE_API UFPTSensesComponent : public UActorComponent
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Detectable Actors") //The type of Actor (and its children) this component can see (otherwise, it's ignored)
	TArray<TSubclassOf<AActor>> DetectableActors;
	//=====VISION====
protected:
	UPROPERTY(EditAnywhere,Category = "Vision") //Disable this for blind enemies! (And performance, as it stops the Tick from running)
	bool bCanSee;
	UPROPERTY(EditAnywhere, Category = "Vision", meta = (UIMin = 0, ClampMin = 0)) //This is how far we can see
	int32 MaxSightDistance;
	UPROPERTY(EditAnywhere, Category = "Vision", meta = (UIMin = 0, ClampMin = 0, UIMax = 180, ClampMax = 180))
	uint8 PeripheralVision;
	UPROPERTY() //All the current actors that are within our field of sight
	TArray<AActor*> VisibleActors;
	


	//=======================================================================================
	//=======================================FUNCTIONS=======================================
	//=======================================================================================

public:	
	// Sets default values for this component's properties
	UFPTSensesComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	UFUNCTION() //Visuals
	virtual void VisualTick(float DeltaTime);


	//======================
	//========VISION========
	//======================
protected:
	UFUNCTION() //Used to check to see if the actor is apart of a class inside our DetectableActors array
	bool IsValidDetectableClass(TSubclassOf<AActor> ActorClass);
public:
	UFUNCTION(BlueprintCallable,Category = "Senses") //Can we currently see this actor?
	bool CanSee(AActor* ActorToCheck);
	UFUNCTION() //Returns true if this actor is apart of ActorsInSight
	bool IsActorInSight(AActor* ActorToCheck);
protected:
	UFUNCTION()
	void AddToVisibleList(AActor* NewActor);
	UFUNCTION()
	void RemoveFromVisibleList(AActor* RemovableActor);
	UFUNCTION() //Returns how far we can CURRENTLY see based off of conditional things!
	int32 ReturnVisualRange();

		
	
};
