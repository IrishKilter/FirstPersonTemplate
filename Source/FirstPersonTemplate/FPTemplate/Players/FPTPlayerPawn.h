// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FPTemplate/FPTCharacter.h"
#include "FPTCharacterMovementComponent.h"
#include "FPTPlayerPawn.generated.h"

//Our player's Stance
//We need to declare ENUMS outside of the UCLASS 
UENUM(BlueprintType) //Allows this ENUM to show up in Blueprints as a list
enum class EStance : uint8 //Declares a new type of ENUM
{
	ENUM_Standing UMETA(DisplayName = "Standing"), //UMETA allows us to make a Friendly Display Name
	ENUM_Crouched UMETA(DisplayName = "Crouched"),
	ENUM_Prone UMETA(DisplayName = "Prone")
};

UENUM()
enum class ECameraType : uint8
{
	ENUM_Traditional UMETA(DisplayName = "Traditional First Person"), //Loosely places the camera in range
	ENUM_True UMETA(DisplayName = "True First Person") //Attaches the camera to the eye location socket
};


/*
UCLASS()
class FIRSTPERSONTEMPLATE_API AFPTPlayerPawn : public AFPTCharacter
{
	GENERATED_BODY()




};*/


UCLASS()
class FIRSTPERSONTEMPLATE_API AFPTPlayerPawn : public AFPTCharacter
{
	GENERATED_BODY()


		//===============================
		//===========INVENTORY===========
		//===============================
protected:
	UPROPERTY(Replicated)
		class AFPTInventoryManager* InventoryManager;
	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
		TSubclassOf<class AFPTInventoryManager> InventoryManagerClass;

	UPROPERTY(ReplicatedUsing = OnRep_UpdateInHand) //The item we currently have selected
		class AFPTInventoryItem* InHand;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
		FName RightHandSocketName;
	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
		FName LeftHandSocketName;

	UPROPERTY(EditAnywhere, Category = "Inventory")
		TArray<TSubclassOf<class AFPTInventoryItem>> DefaultInventory;

	//===============================
	//============WEAPONS============
	//===============================
	UPROPERTY(meta = (ClampMin = 0, ClampMax = 100))
		float PlayerAccuracy; //Our player's current accuracy rating 0-100%

	 //==============================
	 //===========MOVEMENT===========
	 //==============================

private:
	UPROPERTY(ReplicatedUsing = OnRep_UpdateRotation) //Replicate the character's rotation to all clients
		FRotator CharacterRotation;
	//Stances
	UPROPERTY(Replicated)
		EStance Stance;

	UPROPERTY(EditAnywhere, Category = "Character Movement (Rotation Settings)", meta = (UIMin = 0, UIMax = 90)) //How much our character can pitch up
		uint8 CharacterPitchUpLimit;
	UPROPERTY(EditAnywhere, Category = "Character Movement (Rotation Settings)", meta = (UIMin = 0, UIMAx = 90)) //How much our character can pitch down
		uint8 CharacterPitchDownLimit;
	UPROPERTY(Replicated) //How much our character is aiming up/down
		float CharacterPitch;

	//============================
	//===========CAMERA===========
	//============================
protected:
	UPROPERTY(EditAnywhere, Category = "Camera") //The camera component
		UCameraComponent* Camera;
	UPROPERTY(EditDefaultsOnly, Category = "Camera") //Type of Camera we're using
		ECameraType CameraStyle;
	//We can't use UPROPERTY() on FTimerHandle types, it'll cause a compiler failure
	FTimerHandle EyeHeightTimer; //Timer Handler used to handle the timing of the eye height change (for Traditional Camera Style)


	 //==========================================================================================================================================================================================
	 // ============================================================================ FUNCTIONS ==================================================================================================
	 //==========================================================================================================================================================================================

public:
	// Sets default values for this character's properties
	AFPTPlayerPawn(const FObjectInitializer& ObjectInitialzer);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


	//=========================================
	//=============INVENTORY===================
	//=========================================

	//Inventory Manager
protected:
	UFUNCTION() //Called by the server to create and replicate the Inventory Manager for this pawn
		void InitInventoryManager();
public:
	UFUNCTION() //Returns the reference to the Inventory Manager's default class
		TSubclassOf<class AFPTInventoryManager> ReturnInventoryManagerClass() { return InventoryManagerClass; }
	UFUNCTION(BlueprintCallable, Category = "Inventory")
		class AFPTInventoryManager* ReturnInventoryManager() { return InventoryManager; }

	//Default Inventory
protected:
	UFUNCTION() //Spawns the default inventory for the player when starting up
		void CreateDefaultInventory();

	//Inventory Handling
public:
	UFUNCTION(BlueprintCallable, Category = "Inventory")
		class AFPTInventoryItem* ReturnInHand() { return InHand; }
	UFUNCTION(BlueprintCallable, Category = "Inventory")
		void EquipItem(class AFPTInventoryItem* Item);
	UFUNCTION(BlueprintCallable, Category = "Inventory")
		void UnEquipItem(class AFPTInventoryItem* Item);
protected:
	UFUNCTION() //Attaches the item to the Pawn
		void AttachItem(class AFPTInventoryItem* Item);

protected:
	UFUNCTION() //Updates players about what this pawn is currently holding
		void OnRep_UpdateInHand();


	//==========================================
	//==================WEAPONS==================
	//==========================================

	//Firing
protected:
	UFUNCTION()
		void Fire();
	UFUNCTION()
		void ReleaseFire();
	//Aiming
	UFUNCTION() //Raises the weapon
		void AimDownSights();
	UFUNCTION() //Lowers the weapon back to the hip
		void ReleaseAim();
	//Weapon Functions
	UFUNCTION()
		void ToggleFireMode();

	//Accuracy
protected:
	UFUNCTION()//Calculates the Player's current accuracy level (0-100%) based on their current state (stamina, injuries,stance,etc)
		void CalcPlayerAccuracy(float DeltaTime);
public:
	UFUNCTION() //Returns the Player's Accuracy
		float ReturnPlayerAccuracy();
	//========================================
	//=============MOVEMENT===================
	//========================================
protected:
	UFUNCTION() //Applies Forward/Backwards movement VIA the MoveForward and MoveBackwards key bindings
		virtual void MoveForward(float Value);
	UFUNCTION()//Applies Left/Right strafing movement VIA the MoveRight and MoveLeft key bindings
		virtual void MoveRight(float value);
	UFUNCTION() //Receives the rotation input from our binding
		virtual void Rotate(float Value);
	UFUNCTION(Server, Reliable, WithValidation) //Tells the server which way we're facing
		virtual void Server_UpdateRotation(float NewRotation);
	UFUNCTION() //Tells all non-owning clients which way we're facing
		void OnRep_UpdateRotation();
	UFUNCTION() //receives the pitch input from out binding
		virtual void Pitch(float Value);
	UFUNCTION(Server, Reliable, WithValidation) //Tells the server which way we're pitching up/down for replicating shooting/aiming
		virtual void Server_UpdatePitch(float NewPitch);

	//=== Crouching ===
protected:
	UFUNCTION() //Exec to stand the player from either crouching or prone
		virtual void Stand();
	UFUNCTION() //Exec to crouch the player 
		void BeginCrouch();
	UFUNCTION(Server, Reliable, WithValidation) //This let's the server know we've changed our stance to a crouch
		void Server_Crouch();
	UFUNCTION() //Exec called when the player releases the crouch button
		void CrouchRelease();
	UFUNCTION(Server, Reliable, WithValidation)
		void Server_CrouchRelease();
	UFUNCTION() //Exec to lay down/go prone
		void GoProne();



public:
	UFUNCTION(BlueprintCallable, Category = "Stance") //Returns what our current stance is
		EStance ReturnStance() { return Stance; }
private:
	UFUNCTION() //Changes the Stance ENUM which is used for animations
		void SetStance(EStance NewStance);


	//=====================================
	//=============CAMERA==================
	//=====================================
public:
	virtual void CalcCamera(float DeltaTime, struct FMinimalViewInfo& OutResult) override;

	UFUNCTION() //Gives us the reference to our Camera Component
		UCameraComponent* ReturnCameraComponent() { return Camera; }
	UFUNCTION() //Tells us what type of camera style we're using. Traditional or True first person
		ECameraType ReturnCameraStyle() { return CameraStyle; }

	UFUNCTION() //A looping function called from a timer that blends our height changes when changing stances if using Traditional camera style
		void LerpEyeHeight();


};
