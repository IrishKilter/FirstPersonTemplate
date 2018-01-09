// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "FPTemplate/Inventory/FPTInventoryItem.h"
#include "FPTWeapon.generated.h"


//The fire mode for our weapon (Semi, Burst, Full-Auto)
UENUM(BlueprintType) //Allows the ENUM to showup in Blueprints
enum class EFireMode : uint8
{
	ENUM_Semi UMETA(DisplayName = "Semi-Auto"),
	ENUM_Burst UMETA(DisplayName = "Burst"),
	ENUM_FullAuto UMETA(DisplayName = "Full-Auto")
};


/**
 * 
 */
UCLASS(abstract,HideCategories=("Actor Tick",Rendering,Replication,Input,Actor))
class FIRSTPERSONTEMPLATE_API AFPTWeapon : public AFPTInventoryItem
{
	GENERATED_BODY()
	
	//====================
	//========Snuff=======
	//====================
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	uint8 MaxDamage;
	
	//Firing
protected:
	UPROPERTY()
	bool bTriggerDepressed;
private:
	UPROPERTY(EditDefaultsOnly, Category = "Firing") //Current FireMode of the Weapon
	EFireMode FireMode;
	UPROPERTY(EditDefaultsOnly, Category = "Firing")
	bool bHasBurstMode;
	UPROPERTY(EditDefaultsOnly, Category = "Firing")
	bool bHasFullAutoMode;
	UPROPERTY(EditDefaultsOnly,Category = "Firing") //Bullets Per Second/How fast we fire)
	float FireRate;
	UPROPERTY(EditDefaultsOnly, Category = "Firing",meta=(EditCondition="bHasBurstMode")) //Bullets Per/Second for Burst Mode
	float BurstFireRate; 
	UPROPERTY()
	uint8 BurstRoundCounter; //Counter used to keep track of the rounds we've fired so far in our Burst Mode
	UPROPERTY(EditDefaultsOnly, Category = "Firing",meta=(EditCondition="bHasBurstMode")) //How many rounds are fired total in a burst
	uint8 RoundsPerBurst;
	UPROPERTY()
	float FireTimeStamp; //In Seconds, the TimeStamp of when we first started firing this current round
	FTimerHandle FireRateTimer;
	UPROPERTY(EditDefaultsOnly, Category = "Accuracy") //The max distance our shot can travel
	float MaxRange;
	UPROPERTY()
	float BulletSpread; //The current amount of bullet spread (This value is (Amount/10) to allow for better UI controls. IE: 100 = 10 Degrees, 50 = 5 Degrees;
	UPROPERTY(EditDefaultsOnly, Category = "Accuracy",meta=(UIMin=0,ClampMin=0,UIMax=100,ClampMax=100)) //The MAX bullet spread cone size (Amount/10) IE: 100 = 10 Degrees, 50 = 5 Degrees
	float MaxBulletSpread;
	UPROPERTY(EditDefaultsOnly, Category = "Accuracy", meta = (UIMin = 0, ClampMin = 0, UIMax = 100, ClampMax = 100)) //The MIN bullet spread cone size
	float MinBulletSpread;
	UPROPERTY(EditDefaultsOnly, Category = "Accuracy", meta = (UIMin = 0, ClampMin = 0, UIMax = 100, ClampMax = 100)) //How long it takes to reach MAX bullet spread (in seconds)
	float SpreadRate;
	UPROPERTY(EditDefaultsOnly, Category = "Accuracy", meta = (UIMin = 0, ClampMin = 0, UIMax = 100, ClampMax = 100)) //How long it takes after the last shot is fired, to begin reducing the spread from the recoil
	float SpreadCooldown;
	UPROPERTY(EditDefaultsOnly, Category = "Accuracy", meta = (UIMin = 0, ClampMin = 0, UIMax = 100, ClampMax = 100)) //Once cooldown begins, how long does it take to bring weapon spread back to 0
	float SpreadCooldownRate;
	



	//Mesh Info
protected:
	UPROPERTY(EditAnywhere, Category = "Mesh")
	USkeletalMeshComponent* Mesh;

	//Camera
public:
	UPROPERTY(EditDefaultsOnly, Category = "Camera") //First person mesh Camera Offset (for Traditional View)
	FVector MeshCameraOffset;
	UPROPERTY(EditDefaultsOnly, Category = "Camera") //The first person camera offset when aiming down sights
	FVector SightsCameraOffset;
	UPROPERTY(EditDefaultsOnly, Category = "Camera") //The amount the camera zooms in when aiming
	float SightsFOV;
	UPROPERTY(EditDefaultsOnly, Category = "Camera") //The camera animation for when a shot is fired
	TSubclassOf<UCameraShake> RecoilCameraAnim;
	UPROPERTY(EditDefaultsOnly, Category = "Camera", meta = (EditCondition="bHasBurstMode")) //If set, overrides FireCameraShake whenever in Burst Mode
	TSubclassOf<UCameraShake> BurstRecoilCameraAnim;


	//Sound Effects
	UPROPERTY(EditDefaultsOnly, Category = "Sound Effects") //Pew Pew Pew
	USoundCue* FireSound;
	UPROPERTY(EditDefaultsOnly, Category = "Sound Effects") //Empty Firing
	USoundCue* DryFireSound; 



	//===============================================================
	//===========================FUNCITONS===========================
	//===============================================================

public:
	AFPTWeapon(const FObjectInitializer& ObjectInitializer);
	
	//======================
	//======ATTACHMENT======
	//======================
public:
	virtual void OnEquipped_Implementation() override;
	virtual void OnUnequipped_Implementation() override;
	
	//======================
	//=========FIRING=======
	//======================
	UFUNCTION()
	void BeginFire();
protected:
	UFUNCTION(Server, Reliable, WithValidation)
	void RequestFire();
public:
	UFUNCTION()
	void EndFire();
protected:
	UFUNCTION(Server, Reliable, WithValidation)
	void RequestEndFire();
protected:
	UFUNCTION()
	void PerformShot();
	UFUNCTION()
	void CalcBulletSpread();
	UFUNCTION()
	float ReturnAimError();
public:
	UFUNCTION()
	bool CanFire();
	UFUNCTION(BlueprintCallable, Category = "Firing") //Returns the current firemode of this weapon
	EFireMode ReturnFireMode();
	UFUNCTION()
	void ToggleFireMode();
	
protected:
	UFUNCTION()
	uint8 ReturnWeaponDamage();


	//==========================
	//========ANIMATIONS========
	//==========================
protected:
	UFUNCTION()
	void PlayRecoilAnim();

	//======================
	//========SOUNDS========
	//======================

protected:
	UFUNCTION()
	void PlayFireSound();
};
