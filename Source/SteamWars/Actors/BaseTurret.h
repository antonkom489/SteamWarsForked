#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseTurret.generated.h"

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

private:
	FTimerHandle TimerHandle;
	
	bool Captured = false;
	FCollisionQueryParams CollisionParams;

	TWeakObjectPtr<AActor> CaptureActor;
	
	float TargetRotationAngle;
	float CurrentRotationAngle;
	
	void TurretUpdate();
	void RotateTurret();
};
