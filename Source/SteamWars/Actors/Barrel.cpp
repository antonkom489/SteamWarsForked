#include "Barrel.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"

ABarrel::ABarrel()
{
	PrimaryActorTick.bCanEverTick = false;
	CurrentRadius = 0.0f;

	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	RootComponent = SphereComponent;
	
	Basement = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Basement"));
	Basement->SetRelativeLocation(FVector(0, 0, 0));
}

void ABarrel::BeginPlay()
{
	Super::BeginPlay();
}

void ABarrel::DamageBarrel(float damage)
{
	HP-=damage;
	if(HP<=0)
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, this,
			&ABarrel::ExploseBarrel, 0.1f, true);
}

void ABarrel::ExploseBarrel()
{
	if (CurrentRadius < ExploseRadius)
	{
		CurrentRadius += ExpandSpeed * 0.1f;
		SphereComponent->SetSphereRadius(FMath::Min(CurrentRadius, ExploseRadius));
	}
	else
	{
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
	}
}

void ABarrel::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit) const
{
	if (OtherActor && OtherActor != this)
	{ }
}
