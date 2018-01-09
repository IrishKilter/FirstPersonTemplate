// Fill out your copyright notice in the Description page of Project Settings.

#include "FirstPersonTemplate.h"
#include "../Gameplay/FPTDamageInterface.h"
#include "FPTWeapon.h"



AFPTWeapon::AFPTWeapon(const FObjectInitializer& ObjectInitializer)
{
	
	MaxDamage = 70;

	//Firing
	FireMode = EFireMode::ENUM_Semi;
	FireRate = 0.03;
	BurstFireRate = 0.2;
	RoundsPerBurst = 3;
	//Accuracy
	MaxRange = 5000; 
	MaxBulletSpread = 18;
	MinBulletSpread = 8.0;
	BulletSpread = MinBulletSpread; //Just keeps the very first shot from being 100% accurate because we don't calculate it dynamically till after the shot
	SpreadRate = 0.75f;
	SpreadCooldown = 0.25f;
	SpreadCooldownRate = 0.5f;
	

	MeshCameraOffset = FVector(-5.5,22.2,-25.0);
	SightsCameraOffset = FVector(-6.255005, 0, -13.906677);
	SightsFOV = 72;
	
	Mesh = ObjectInitializer.CreateDefaultSubobject<USkeletalMeshComponent>(this, TEXT("MeshComp"));
	if (Mesh)
	{
		SetRootComponent(Mesh);
	}
}

//==============================
//==========ATTACHMENT==========
//==============================

/* OnEquipped() - Called after a successful equip from the Pawn
*
*
*/
void AFPTWeapon::OnEquipped_Implementation()
{
	if(!GetOwner())
		return;

}


void AFPTWeapon::OnUnequipped_Implementation()
{

}

//===================================
//===========FIRING==================
//===================================

/* BeginFire() 
* The owning controller gets to attempt to fire on their end, but only sends the important information to the server based on their version
* The server then confirms their info, and does the actual calculations to perform the shot, and replicates it to all clients
*
*/
void AFPTWeapon::BeginFire()
{
	if (CanFire()) //If we can fire
	{
		if (!bTriggerDepressed) //If we haven't already begun to fire
		{		
			if(!HasAuthority())//If we're a client
				RequestFire(); //Tell the server we've begun to fire (we want this to only be called once to reduce packages sent)
			
			bTriggerDepressed = true; //Depress the trigger to keep track of our firing and to avoid constantly flooding the server with fire requests

			if (HasAuthority()) //We're the server
				FireTimeStamp = GetWorld()->GetTimeSeconds();
		}		
		
		if (HasAuthority()) //If we're the server
		{
			PerformShot(); //Perform the actual shot calculations
			CalcBulletSpread(); //Calculate bullet spread from the recoil after every shot
		}

		//If we're in a full-auto mode, let's continue to fire by looping back to the beginning of the BeginFire() function
		if (ReturnFireMode() == EFireMode::ENUM_FullAuto) //Are we in Full-Auto?
		{
			if (!GetWorldTimerManager().IsTimerActive(FireRateTimer)) //If our timer isn't active, begin it
				GetWorldTimerManager().SetTimer(FireRateTimer, this, &AFPTWeapon::BeginFire, FireRate, true); //Use a timer to loop this function via FireRate timing for Automatic Firing with a loop
		}
		//If we're in burst mode
		else if (ReturnFireMode() == EFireMode::ENUM_Burst)
		{
			//Begin the Burst
			if (!GetWorldTimerManager().IsTimerActive(FireRateTimer)) //We haven't started the burst yet
				GetWorldTimerManager().SetTimer(FireRateTimer, this, &AFPTWeapon::BeginFire, BurstFireRate, true); //Start it!
			//Increase the round counter to keep track of our burst
			if (BurstRoundCounter < RoundsPerBurst) //If we haven't reached our limit yet
				BurstRoundCounter++; //+1
		}
		
		//Let's make some fancy effects!
		PlayFireSound(); //Play the sound effect
		PlayRecoilAnim(); //Pushes the camera around a little for effect during shots
	}	
	//Can't fire!
	else
	{
		if (bTriggerDepressed)
			EndFire();
	}
}

bool AFPTWeapon::RequestFire_Validate()
{
	return true;
}

void AFPTWeapon::RequestFire_Implementation()
{
	BeginFire();
}

void AFPTWeapon::EndFire()
{		
	if(!HasAuthority()) //If we're a client
		RequestEndFire(); //Tell the server we've stopped firing
	
	bTriggerDepressed = false; //End the firing loop locally

	//If we're in Burst mode, let's make sure we've finished our entire burst before clearing out
	if (ReturnFireMode() == EFireMode::ENUM_Burst && BurstRoundCounter)
	{
		if (BurstRoundCounter >= RoundsPerBurst) //If we've finished our burst
		{			
			BurstRoundCounter = 0; //Reset the Burst Counter
			if (GetWorldTimerManager().IsTimerActive(FireRateTimer)) //If the timer is active
				GetWorldTimerManager().ClearTimer(FireRateTimer); //Clear the firing timer
		}
	}
	//Any other FireMode
	else
	{
		if (GetWorldTimerManager().IsTimerActive(FireRateTimer))
			GetWorldTimerManager().ClearTimer(FireRateTimer); //Clear the firing timer
	}
}


bool AFPTWeapon::RequestEndFire_Validate()
{
	return true;
}

void AFPTWeapon::RequestEndFire_Implementation()
{
	EndFire();
}

void AFPTWeapon::PerformShot()
{
	if (!HasAuthority()) //Prevent anyone other than the server from executing this
		return;
	
	
	//==== Setup Trace====
	//Get Camera View
	FMinimalViewInfo CameraView;
	Cast<AFPTPlayerPawn>(ReturnOwningPawn())->ReturnCameraComponent()->GetCameraView(FApp::GetDeltaTime(), CameraView); //Pull camera view from the Player
	//Setup Trace Information
	FVector TraceStart = CameraView.Location; //TraceStart begins with out Camera
	FVector TraceDir = CameraView.Rotation.Vector();
	FVector TraceEnd = TraceStart + (FMath::VRandCone(TraceDir, FMath::DegreesToRadians(ReturnAimError())) * MaxRange); //Takes the TraceStart and TraceDirection, and applies aim error to it, then extends the Trace's End based on Weapon's Range
	
    //FVector TraceEnd = TraceStart + (CameraView.Rotation.Vector() * MaxRange); //Get Camera Directional Vector, and then apply the MaxRange to it
	
	FHitResult HitResult;
	FCollisionQueryParams CollisionParams;
	//Disable collision for us and our owning pawn
	CollisionParams.AddIgnoredActor(this);
	CollisionParams.AddIgnoredActor(ReturnOwningPawn());
	
	//==== Run our trace to see if we hit anything ====
	//@ECC_GameTraceChannel1 - this is our custom Trace Channel (Weapon). After creating a custom channel, it appears inside the Collision section of the DefaultEngine.ini for us to reference to

	if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECollisionChannel::ECC_GameTraceChannel1, CollisionParams)) //Check to see if we hit anything
		TraceEnd = HitResult.Location; //Setup trace information
		
		//We hit an actor that can take damage
		if (HitResult.GetActor() != nullptr && HitResult.GetActor()->IsValidLowLevel() && HitResult.GetActor()->GetClass()->ImplementsInterface(UFPTDamageInterface::StaticClass()))
		{
			UE_LOG(LogFPT, All, TEXT("!!!HIT!!! - %s"), *HitResult.GetActor()->GetName());
			IFPTDamageInterface::Execute_ReceiveDamage(HitResult.GetActor(), ReturnWeaponDamage(),HitResult,this,ReturnOwningPawn()); //Need to use the EXECUTE_ version to prevent crashing (due to stuff I don't understand)
		}
		else //We hit an actor that can not take damage
		{

		}

	//==== Draw Trace Debug Lines ====
	if (HitResult.bBlockingHit) //We hit something
	{
		DrawDebugLine(GetWorld(), TraceStart, HitResult.Location, FColor::Yellow, false, 1);
		DrawDebugSphere(GetWorld(), HitResult.Location, 4, 24, FColor::Red, false, 1);
	}
	else //We didn't hit anything
	{
		DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Yellow, false, 1);
	}
}

/* CalcBulletSpread() - Called after ever shot, increasing the Bullet Spread amount due to recoil
*
*
*/
void AFPTWeapon::CalcBulletSpread()
{
	//Basic initial setup, keep BulletSpread always at MinBulletSpread until recoil is factored in
	BulletSpread = MinBulletSpread;
	//Add bullet spread based on how long we've been firing	
	float FireTime = GetWorld()->GetTimeSeconds() - FireTimeStamp; //How long we've been firing
	

	float FireTimePercentage = FMath::Clamp(((FireTime * 100) / SpreadRate),0.0f,100.0f); //Finds the percentage that FireTime is of SpreedRate and converts it to a 0-1 range to be used easily 
	FireTimePercentage = FireTimePercentage / 100; //Converts Percentage to a 0.0-1.0 range
	
		
	//Handle Bullet Spread
	if (MaxBulletSpread * FireTimePercentage > MinBulletSpread) //If our BulletSpread has reached past our MinBulletSpread
		BulletSpread = MaxBulletSpread * FireTimePercentage; 
	else //If we haven't hit our minimum Spread yet, just use the Min
		BulletSpread = MinBulletSpread;


}

/* ReturnAimError() - Returns the Aiming Error amount (in degrees) 
*
*
*/
float AFPTWeapon::ReturnAimError()
{
	if (!ReturnOwningPawn()) //Make sure we have a proper Pawn to work with
		return 0;

	//Setup
	float AimError;
	float WeaponBulletSpread = BulletSpread / 10; //We convert the BulletSpread to degrees but dividing by 10. This helps make it pleasing to edit with a range of 0-100, but we keep the actuall degree of offset to 0-10. IE: 100 = 10 Degrees and 50 = 5 Degrees
	//float PlayerAccuracy = ReturnOwningPawn()->ReturnPlayerAccuracy(); //Get the player's accuracy

	//@Temp - Basic needs right now, BulletSpread is a minimum between 0 degree offset, and current bullet spread

	AimError = WeaponBulletSpread;
	//AimError = FMath::RandRange(0.0f, WeaponBulletSpread);


	return AimError;

}

bool AFPTWeapon::CanFire()
{
	//If we're in burst mode, and already reached the burst amount stop us from firing more
	if (ReturnFireMode() == EFireMode::ENUM_Burst && BurstRoundCounter >= RoundsPerBurst)
		return false;

	//if (ClipSize <= 0)
	//	return false;

	return true;
}

/* ReturnFireMode() - Returns the EFireMode ENUM this weapon is currently set to
*
*
*
*/

EFireMode AFPTWeapon::ReturnFireMode()
{
	return FireMode;
}

/* ToggleFireMode() - Cycles through the available fire modes
*
*
*
*/

void AFPTWeapon::ToggleFireMode()
{
	UE_LOG(LogFPT, All, TEXT("%s::FireModeToggle()"),*GetName());
	
	if(ReturnFireMode() == EFireMode::ENUM_Semi)
	{
		if(bHasBurstMode)
			FireMode = EFireMode::ENUM_Burst;
		else if (bHasFullAutoMode)
			FireMode = EFireMode::ENUM_FullAuto;
		else
			return;
	}
	
	else if(ReturnFireMode() == EFireMode::ENUM_Burst)
	{
		if (bHasFullAutoMode)
			FireMode = EFireMode::ENUM_FullAuto;
		else
			FireMode = EFireMode::ENUM_Semi;
	}

	else if (ReturnFireMode() == EFireMode::ENUM_FullAuto)
	{
		FireMode = EFireMode::ENUM_Semi;
	}
	//Debug!
	if(ReturnFireMode() == EFireMode::ENUM_Semi)
		UE_LOG(LogFPT, All, TEXT("Switched To Semi-Auto"));
	if(ReturnFireMode() == EFireMode::ENUM_Burst)
		UE_LOG(LogFPT, All, TEXT("Switched To Burst"));
	if (ReturnFireMode() == EFireMode::ENUM_FullAuto)
		UE_LOG(LogFPT, All, TEXT("Switched To Full-Auto"));
}

//==========================================
//==================DAMAGE==================
//==========================================

uint8 AFPTWeapon::ReturnWeaponDamage()
{
	return FMath::RandRange(MaxDamage * 0.8,MaxDamage); //Gives a little bit of range
}

//============================================
//=================ANIMATIONS=================
//============================================

void AFPTWeapon::PlayRecoilAnim()
{
	//Only play this for the local controller
	if (!ReturnOwningPawn()->IsLocallyControlled())
		return;

	if (ReturnFireMode() == EFireMode::ENUM_Burst && BurstRecoilCameraAnim)
	{
		if (ReturnOwningPawn() && Cast<APlayerController>(ReturnOwningPawn()->GetController()))
			Cast<APlayerController>(ReturnOwningPawn()->GetController())->ClientPlayCameraShake(BurstRecoilCameraAnim);
	}

	else
	{
		if (RecoilCameraAnim && ReturnOwningPawn() && Cast<APlayerController>(ReturnOwningPawn()->GetController()))
			Cast<APlayerController>(ReturnOwningPawn()->GetController())->ClientPlayCameraShake(RecoilCameraAnim);
	}
		
}



//=========================================
//==============SOUNDS=====================
//=========================================

void AFPTWeapon::PlayFireSound()
{
	if(FireSound)
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), FireSound, GetActorLocation());
}

