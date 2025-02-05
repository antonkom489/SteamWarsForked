#pragma once

#include "CoreMinimal.h"
#include "Components/ArrowComponent.h"
#include "GameFramework/Actor.h"
#include "Perception/PawnSensingComponent.h"
#include "BaseTurret.generated.h"

UENUM(Blueprintable)
enum class ETurretState : uint8
{
	Fire,
	Looking
};

UCLASS()
class STEAMWARS_API ABaseTurret : public AActor
{
	GENERATED_BODY() 

public:
	ABaseTurret();

protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* Basement;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components") 
	UStaticMeshComponent* Gun;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UPawnSensingComponent* PawnSensingComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UArrowComponent* ArrowComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	float Health = 100;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	float SphereRadius = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	float RangeOfVisibility = 800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	float MaxRotationAngle = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	float RotationSpeed = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	float MinPitch = -30.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	float MaxPitch = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	float HearingRange = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	float ShootDelay = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	ETurretState turretState = ETurretState::Looking;

	FTimerHandle ShootTimerHandle;
	
	UFUNCTION()
	void OnSeePawn(APawn* SeenPawn);
	
private:
	FTimerHandle TimerHandle;
	
	bool Captured = false;
	
	FCollisionQueryParams CollisionParams;
	
	TWeakObjectPtr<AActor> CaptureActor;
	
	float TargetRotationAngle;
	float CurrentRotationAngle;
	
	//void TurretUpdate();
	void RotateTurret();
	void LookMode();
	void AttackMode();
	void Shoot();
	void StartShoot();
	void EndShoot();
};

