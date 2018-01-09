// Fill out your copyright notice in the Description page of Project Settings.

#include "FirstPersonTemplate.h"
#include "../Inventory/FPTInventoryManager.h"
#include "../Inventory/FPTInventoryItem.h"
#include "../Inventory/FPTWeapon.h"
#include "FPTPlayerPawn.h"


void AFPTPlayerPawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	//Call all of the parents replicated construction
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//Inventory
	DOREPLIFETIME_CONDITION(AFPTPlayerPawn, InventoryManager, COND_OwnerOnly);
	DOREPLIFETIME(AFPTPlayerPawn, InHand);
	//Movement Snuff
	DOREPLIFETIME(AFPTPlayerPawn, Stance);
	DOREPLIFETIME_CONDITION(AFPTPlayerPawn, CharacterRotation, COND_SkipOwner); //The owning client does their version on their own
}

// Sets default values
AFPTPlayerPawn::AFPTPlayerPawn(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer.SetDefaultSubobjectClass<UFPTCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	//=== Inventory ===
	InventoryManagerClass = AFPTInventoryManager::StaticClass();

	RightHandSocketName = "RightHandSocket";
	LeftHandSocketName = "LeftHandSocket";

	//=== Movement ===

	//Disables the Controller from controlling our Pawn's rotation
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;

	if (GetCharacterMovement()) //Make sure we have a valid Character Movement
	{
		GetCharacterMovement()->MaxWalkSpeed = 450; //Standard Walking Speed
	}

	//Leaning
	CharacterPitchUpLimit = 90;
	CharacterPitchDownLimit = 70;

	//=== Mesh Setup ===
	if (GetMesh()) //Make sure we have a valid Mesh
	{
		//Find our asset via a path && create a reference to it
		static ConstructorHelpers::FObjectFinder<USkeletalMesh> TempMesh(TEXT("/Game/Template/Characters/Placeholder_Human/Mesh/SK_PlaceHolder_Male.SK_PlaceHolder_Male"));

		GetMesh()->SkeletalMesh = (USkeletalMesh*)TempMesh.Object; //Set the skeletal mesh to the reference of our mesh
		GetMesh()->RelativeLocation = FVector(0, 0, -90); //Adjust our mesh location so the feet properly touch the ground
		if (ReturnCameraStyle() == ECameraType::ENUM_Traditional) //If we're using Traditional First Person Views
		{
			GetMesh()->bOwnerNoSee = true; //Hide the 3rd person mesh from the owning player
		}
	}

	//=== Animation Setup ===
	//Find our asset via a path && create a reference to it
	//static ConstructorHelpers::FObjectFinder<UAnimBlueprint> AnimObj(TEXT("AnimBlueprint'/Game/Template/Characters/PlayerBase/Animations/PlayerPlaceholder-AnimBlueprint.PlayerPlaceholder-AnimBlueprint'"));
	/*
	if (AnimObj.Object) //make sure we have a valid Animaiton Reference
	{
	if (GetMesh()) //Make sure there is a Mesh to set to
	GetMesh()->SetAnimInstanceClass(AnimObj.Object->GetAnimBlueprintGeneratedClass()); //Apply the Animation Reference to our Animation Blueprint inside our Mesh
	}*/


	//=== Camera Setup ===

	CameraStyle = ECameraType::ENUM_Traditional; //Default Camera style
	BaseEyeHeight = 160; //Standing eye height (used only in Traditional CameraStyle)
	CrouchedEyeHeight = 120; //Crouched eye height (used only in Traditional CameraStyle)


							 //Create our default camera component (AKA: Sub-Object) to be used with this instance
	Camera = ObjectInitializer.CreateDefaultSubobject<UCameraComponent>(this, TEXT("CameraComp"));

	if (Camera) //Check for valid Camera reference
	{
		Camera->SetupAttachment(GetMesh());
		Camera->SetRelativeLocation(FVector(35, 0, BaseEyeHeight));
		Camera->FieldOfView = 100;	 //Set the FOV (viewing angle) for this camera	
	}


	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//=== Replication === 
	bReplicates = true; //This enables replication for this class
	bReplicateMovement = true;	//This replicates the location of our instance

}

/* BeginPlay() - Called after the Game Starts for the first time allowing us to modify the Pawn based on game-start conditions
*
*
*
*/
void AFPTPlayerPawn::BeginPlay()
{
	Super::BeginPlay();


	//Server setups
	if (HasAuthority())
	{
		InitInventoryManager(); //Gets the Inventory Manager setup
		CreateDefaultInventory(); //Spawns default inventory for our character (if we have any)
	}
}

/* Tick() - Called every frame.
* IE: If we're running at 60 frames per second, this is called 60 times per second.
* @WARNING:: Only put what ABSOLUTELY NEEDS to be running every frame (physics, movement, major things!!!). Use a Timer for less important things
*/
void AFPTPlayerPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (HasAuthority() || IsLocallyControlled())
		CalcPlayerAccuracy(DeltaTime);

}

/* SetupPlayerInputComponent() - Used to bind key input to functions
*
*
*/
void AFPTPlayerPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	//Weapons
	InputComponent->BindAction("Fire", EInputEvent::IE_Pressed, this, &AFPTPlayerPawn::Fire);
	InputComponent->BindAction("Fire", EInputEvent::IE_Released, this, &AFPTPlayerPawn::ReleaseFire);
	InputComponent->BindAction("AimDownSights", EInputEvent::IE_Pressed, this, &AFPTPlayerPawn::AimDownSights);
	InputComponent->BindAction("AimDownSights", EInputEvent::IE_Released, this, &AFPTPlayerPawn::ReleaseAim);
	InputComponent->BindAction("ToggleFireMode", EInputEvent::IE_Pressed, this, &AFPTPlayerPawn::ToggleFireMode);

	//Movement
	InputComponent->BindAxis("MoveForward", this, &AFPTPlayerPawn::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &AFPTPlayerPawn::MoveRight);
	//Looking
	InputComponent->BindAxis("Rotate", this, &AFPTPlayerPawn::Rotate);
	InputComponent->BindAxis("Pitch", this, &AFPTPlayerPawn::Pitch);
	//Stance
	InputComponent->BindAction("Crouch", EInputEvent::IE_Pressed, this, &AFPTPlayerPawn::BeginCrouch);
	InputComponent->BindAction("Crouch", EInputEvent::IE_Released, this, &AFPTPlayerPawn::CrouchRelease);

	//Jumping
	InputComponent->BindAction("Jump", EInputEvent::IE_Pressed, this, &AFPTPlayerPawn::Jump);

}

//=========================================
//================INVENTORY================
//=========================================

void AFPTPlayerPawn::InitInventoryManager()
{
	UE_LOG(LogFPT, All, TEXT("%s::InitInventoryManager()"), *GetName());
	//Bugger out if we're the client by any chance
	if (!HasAuthority() || GetNetMode() >= ENetMode::NM_Client)
		return;
	//or if we don't have a valid class
	if (ReturnInventoryManagerClass() == NULL)
		return;

	InventoryManager = GetWorld()->SpawnActor<AFPTInventoryManager>(ReturnInventoryManagerClass());
	InventoryManager->InitInvManager(this);
}



//===========================================
//=============DEFAULT INVENTORY=============
//===========================================

void AFPTPlayerPawn::CreateDefaultInventory()
{
	UE_LOG(LogFPTInventory, All, TEXT("%s::CreateDefaultInventory()"), *GetName());

	if (!HasAuthority() || GetNetMode() >= ENetMode::NM_Client)
		return;

	if (DefaultInventory.Num() == 0)
	{
		UE_LOG(LogFPTInventory, All, TEXT("No Default Inventory Items - Returning"))
			return;
	}
	UE_LOG(LogFPTInventory, All, TEXT("Creating Default Inventory...%i item(s) found"), DefaultInventory.Num());
	for (int32 i = 0; i < DefaultInventory.Num(); i++)
	{
		UE_LOG(LogFPTInventory, All, TEXT("DefaultInventory[%i] : %s"), i, *DefaultInventory[i].GetDefaultObject()->GetName());
		UE_LOG(LogFPTInventory, All, TEXT("Spawning Item..."));
		//Spawn Item
		AFPTInventoryItem* NewItem = GetWorld()->SpawnActor<AFPTInventoryItem>(DefaultInventory[i]); //Spawn the item
		ReturnInventoryManager()->AddItem(NewItem); //Add it to the player's inventory


		if (InHand == nullptr)
		{
			UE_LOG(LogFPT, All, TEXT("Assigning default In Hand item"));
			EquipItem(NewItem);

		}
	}

	if (DefaultInventory[0])
	{

		//InHand = GetWorld()->SpawnActor<AFPTInventoryItem>(DefaultInventory[0]);

	}
}


//============================================
//=============INVENTORY HANDLING=============
//============================================

void AFPTPlayerPawn::EquipItem(AFPTInventoryItem* Item)
{
	UE_LOG(LogFPT, All, TEXT("%s::EquipItem()"), *GetName());
	if (!Item || !ReturnInventoryManager()) //Cancel out if there is no  item or Inventory Manager to be using to avoid crashing
		return;

	//If we're the server
	if (HasAuthority())
	{

		//Checks to see if the item is registered to our Inventory Manager
		if (!ReturnInventoryManager()->IsItemRegistered(Item))
			ReturnInventoryManager()->RegisterItem(Item);

		//Attach the Item
		AttachItem(Item);

		//Sets the InHand variable to our new item
		InHand = Item;

		if (InHand) //If we were successful!
			InHand->OnEquipped(); //Tell the Item it's been equipped
	}

}

void AFPTPlayerPawn::UnEquipItem(AFPTInventoryItem* Item)
{

}

void AFPTPlayerPawn::AttachItem(AFPTInventoryItem* Item)
{
	UE_LOG(LogFPT, All, TEXT("%s::AttachItem()"), *GetName());

	if (!Item)
	{
		UE_LOG(LogFPT, All, TEXT("!!!! NO ITEM FOUND!!! - !!! RETURNING !!!"));
		return;
	}

	//We're the ones currently controlling this pawn
	if (IsLocallyControlled())
	{
		UE_LOG(LogFPT, All, TEXT("IsLocallyControlled == True"));
		UE_LOG(LogFPT, All, TEXT("Attaching Item to Camera..."))
			if (ReturnCameraComponent()) //Valid camera to work with
			{
				Item->AttachToComponent(ReturnCameraComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
				if (Cast<AFPTWeapon>(Item))
				{
					Item->SetActorRelativeLocation(Cast<AFPTWeapon>(Item)->MeshCameraOffset);
				}
			}
			else //No camera was available for us to attach to
			{
				UE_LOG(LogFPT, All, TEXT("!!!NO CAMERA FOUND!!! - !!!RETURNING!!!"));
				return; //abort
			}


	}
	//Everybody else!
	else
	{

		Item->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, "RightHandSocket");
	}

}

/*
*
*
*/
void AFPTPlayerPawn::OnRep_UpdateInHand()
{
	if (InHand)
	{
		AttachItem(InHand);
	}
}

//==========================================
//==================WEAPONS==================
//==========================================

void AFPTPlayerPawn::Fire()
{
	//Prevent crash if there is no weapon
	if (!ReturnInHand())
		return;

	//Weapon Fire()
	if (Cast<AFPTWeapon>(ReturnInHand())) //If we're holding a weapon
	{
		AFPTWeapon* Weap = Cast<AFPTWeapon>(ReturnInHand());

		Weap->BeginFire();
	}
}

void AFPTPlayerPawn::ReleaseFire()
{
	//Prevents crash if there is no weapon
	if (!ReturnInHand())
		return;

	if (Cast<AFPTWeapon>(ReturnInHand()))
	{
		AFPTWeapon* Weap = Cast<AFPTWeapon>(ReturnInHand());
		Weap->EndFire();
	}
}

//==================
//======AIMING======
//==================

/* AimDownSights()
*
*
*/

void AFPTPlayerPawn::AimDownSights()
{
	if (IsLocallyControlled())
	{

		if (InHand->IsA(AFPTWeapon::StaticClass())) //If we're holding a weapon
		{
			InHand->SetActorRelativeLocation(Cast<AFPTWeapon>(InHand)->SightsCameraOffset); //Offset the gun for aim down sights
			Camera->FieldOfView = Cast<AFPTWeapon>(InHand)->SightsFOV;
		}
	}
}

/* ReleaseAim()
*
*
*/

void AFPTPlayerPawn::ReleaseAim()
{
	if (IsLocallyControlled())
	{
		if (InHand->IsA(AFPTWeapon::StaticClass())) //If we're holding a weapon
		{
			InHand->SetActorRelativeLocation(Cast<AFPTWeapon>(InHand)->MeshCameraOffset); //Return to the original hip view
			Camera->FieldOfView = 100;
		}
	}
}

//===========================
//=====WEAPON FUNCTIONS======
//===========================
/* ToggleFireMode() - Changes the weapon's firing mode (Semi/Burst/Auto)
*
*
*/
void AFPTPlayerPawn::ToggleFireMode()
{
	if (IsLocallyControlled())
	{
		if (InHand->IsA(AFPTWeapon::StaticClass()))
		{
			Cast<AFPTWeapon>(InHand)->ToggleFireMode();
		}
	}
}

//=============================
//=======WEAPON ACCURACY=======
//=============================

/* CalcPlayerAccuracy() - Calculates the current accuracy of the player based on various states that apply a penalty towards our total
* We start with 100% accuracy, and as the system checks for penalties we chip those off the total, resulting in the final value
* Things such as running/sprinting, jumping, or falling recently as well as injuries/health, and character stances will negatively affect the player's accuracy
*/
void AFPTPlayerPawn::CalcPlayerAccuracy(float DeltaTime)
{
	PlayerAccuracy = 100; //Start off with 100% Accuracy

						  //Let's begin to chip it down!
}
/*
*
*
*/
float AFPTPlayerPawn::ReturnPlayerAccuracy()
{
	return PlayerAccuracy;
}

//========================================
//================MOVEMENT================
//========================================

/* MoveForward() - Provides the input to move forward or backwards, allowing us to modify it based on what we need
* A positive value is for Forwrad movement, and a negative value is for backwards movement
*
*/
void AFPTPlayerPawn::MoveForward(float Value)
{
	if (Value == 0 || !GetCharacterMovement()) // Double checks the Value
		return;

	AddMovementInput(GetActorForwardVector(), Value);
}

/* MoveRight() - Provides the input to move Left or Right, allowing us to modify it for what we need!
* Positive value is strafing Right, while negative is for strafing Left
*
*/

void AFPTPlayerPawn::MoveRight(float Value)
{
	if (Value == 0 || !GetCharacterMovement())
		return;

	// add movement in that direction
	AddMovementInput(GetActorRightVector(), Value);
}

/* Rotate() - Rotates the pawn
*
*
*/

void AFPTPlayerPawn::Rotate(float Value)
{
	if (Value == 0)
		return;

	AddActorLocalRotation(FRotator(0, Value * GetCharacterMovement()->RotationRate.Yaw * FApp::GetDeltaTime(), 0));

	//Client tells the server we've rotated recently
	if (IsLocallyControlled() && GetNetMode() == ENetMode::NM_Client) //Make sure we're the owning client
	{
		Server_UpdateRotation(GetActorRotation().Yaw); //Give the server our newest value
		return;
	}

	if (HasAuthority() && GetNetMode() != ENetMode::NM_Client)//If we're the server
		CharacterRotation = GetActorRotation();
}

/* Server_Rotate() - Tells the server our newest rotation values
*
*
*/

bool AFPTPlayerPawn::Server_UpdateRotation_Validate(float NewRotation)
{
	return true;
}

void AFPTPlayerPawn::Server_UpdateRotation_Implementation(float NewRotation)
{
	FRotator NewRot = GetActorRotation();
	NewRot.Yaw = NewRotation;

	SetActorRotation(NewRot);
	CharacterRotation = GetActorRotation();
}

/* OnRep_UpdateRotation() - Sent to all non-owning clients what our
*
*
*/
void AFPTPlayerPawn::OnRep_UpdateRotation()
{
	SetActorRotation(CharacterRotation);
}

/* Pitch() -
*
*
*/

void AFPTPlayerPawn::Pitch(float Value)
{
	if (Value == 0.0f) //Input is constantly being sent, so if it's 0 stop the function flow as nothing happened
		return;

	//Local Client
	if (IsLocallyControlled())
	{
		Value *= GetCharacterMovement()->RotationRate.Pitch * FApp::GetDeltaTime(); //Update the pitch rate based on rotation limits of the Pawn


																					//Adjust our CharacterPitch for Animation/Limitation Calculations
		CharacterPitch += Value;
		//Limit our value to their Max/Min pitch ability
		if (CharacterPitch > 0 && CharacterPitch > CharacterPitchUpLimit) //If we're BEYOND our pitch UP limit
			CharacterPitch = CharacterPitchUpLimit; //Stop the pitch at the limit
		else if (CharacterPitch < 0 && CharacterPitch < (CharacterPitchDownLimit * -1)) //If we're BEYOND our pitch DOWN limit
			CharacterPitch = (CharacterPitchDownLimit * -1); //Stop the pitch at the limit

															 //Update camera component
		if (Camera)
		{
			FRotator NewRotation = GetActorRotation(); //Take the Actor's Rotation to start
			NewRotation.Pitch += CharacterPitch; //then add our Pitch offset

			Camera->SetWorldRotation(NewRotation);
		}
	}

	//Client updates server on our new pitch
	if (!HasAuthority() && IsLocallyControlled())
		Server_UpdatePitch(CharacterPitch);

}


bool AFPTPlayerPawn::Server_UpdatePitch_Validate(float Value)
{
	return true;
}

void AFPTPlayerPawn::Server_UpdatePitch_Implementation(float NewPitch)
{
	CharacterPitch = NewPitch;

	//Update camera component
	if (Camera)
	{
		FRotator NewRotation = GetActorRotation(); //Take the Actor's Rotation to start
		NewRotation.Pitch += CharacterPitch; //then add our Pitch offset

		Camera->SetWorldRotation(NewRotation);
	}
}


void AFPTPlayerPawn::Stand()
{
	if (ReturnStance() == EStance::ENUM_Crouched)
		SetStance(EStance::ENUM_Standing);

	//Return the camera to the standard height if using traditional camera type
	if (ReturnCameraStyle() == ECameraType::ENUM_Traditional)
		GetWorldTimerManager().SetTimer(EyeHeightTimer, this, &AFPTPlayerPawn::LerpEyeHeight, 0.005, true);
}

/* Crouch() -
*
*
*/
void AFPTPlayerPawn::BeginCrouch()
{
	if (ReturnStance() != EStance::ENUM_Crouched)
	{
		SetStance(EStance::ENUM_Crouched);

		if (ReturnCameraStyle() == ECameraType::ENUM_Traditional)
			GetWorldTimerManager().SetTimer(EyeHeightTimer, this, &AFPTPlayerPawn::LerpEyeHeight, 0.005, true);
	}

	//If we're the owning client
	if (IsLocallyControlled() && GetNetMode() == ENetMode::NM_Client)
		Server_Crouch(); //Tell the server we're crouching

}

bool AFPTPlayerPawn::Server_Crouch_Validate()
{
	return true;
}

/* Server_Crouch() - Just sets the code needed for players to get the crouching update
*
*
*/

void AFPTPlayerPawn::Server_Crouch_Implementation()
{
	UE_LOG(LogFPT, All, TEXT("Server_Crouch()"));
	SetStance(EStance::ENUM_Crouched);
}

/* CrouchRelease() - Called when the crouch key is released
*
*
*/

void AFPTPlayerPawn::CrouchRelease()
{
	Stand();

	//if we are the owning client
	if (IsLocallyControlled() && GetNetMode() == ENetMode::NM_Client)
		Server_CrouchRelease(); //Tell the server that we've stood up
}


bool AFPTPlayerPawn::Server_CrouchRelease_Validate()
{
	return true;
}

/* GoProne() -
*
*
*/

void AFPTPlayerPawn::Server_CrouchRelease_Implementation()
{
	SetStance(EStance::ENUM_Standing);
}

void AFPTPlayerPawn::GoProne()
{

}

/* SetStance() - Used to set the Stance ENUM cleanly
*
*
*/
void AFPTPlayerPawn::SetStance(EStance NewStance)
{
	Stance = NewStance;
}


//======================================
//================CAMERA================
//======================================

void AFPTPlayerPawn::CalcCamera(float DeltaTime, struct FMinimalViewInfo& OutResult)
{
	Super::CalcCamera(DeltaTime, OutResult);


	//FRotator NewRotation = GetActorRotation(); //Take the Actor's Rotation to start
	//NewRotation.Pitch += CharacterPitch; //then add our Pitch offset
	//Yaw is handled by the controller

	//Camera->SetWorldRotation(NewRotation);;
}


/* LerpEyeHeight() - A LOOP function called from a Timer that transitions the EyeHeight for the Traditional Style camera
*
*
*/
void AFPTPlayerPawn::LerpEyeHeight()
{
	float DesiredEyeHeight; //Create a local variable we can set so we can keep all the lerping using one variable

	switch (Stance) //Check what eyeheight we should be using
	{
	case EStance::ENUM_Standing: DesiredEyeHeight = BaseEyeHeight; //Standing EyeHeight
		break;
	case EStance::ENUM_Crouched: DesiredEyeHeight = CrouchedEyeHeight; //Crouching EyeHeight
		break;
	default: DesiredEyeHeight = BaseEyeHeight; //Default to Standing incase something goes wrong
		break;
	}

	//Set the EyeHeight Lerp (a LERP is a value in-between the start and the end)
	FVector NewRelativeCamLoc = ReturnCameraComponent()->GetRelativeTransform().GetLocation(); //Gets the relative location (from it's parent) of the Camera
	DesiredEyeHeight = FMath::Lerp(NewRelativeCamLoc.Z, DesiredEyeHeight, 0.05); //Gets the relative Z-axis location of the Camera, and LERPS it with the desired EyeHeight
	NewRelativeCamLoc.Z = DesiredEyeHeight; //Combines the old location with the new Eye Height value
	ReturnCameraComponent()->SetRelativeLocation(NewRelativeCamLoc); //Sets the new relative location	
}

