#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Barrel.generated.h"

class USphereComponent;

UCLASS()
class STEAMWARS_API ABarrel : public AActor
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;
	
public:
	ABarrel();
	
	UPROPERTY(VisibleAnywhere)
	USphereComponent* SphereComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* Basement;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	float HP = 1.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	float ExploseRadius = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	float ExploseDamage = 10.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	float ExpandSpeed = 0.1f;

	void DamageBarrel(float damage);

private:
	FTimerHandle TimerHandle;
	float CurrentRadius;
	
	void ExploseBarrel();
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit) const;
};
