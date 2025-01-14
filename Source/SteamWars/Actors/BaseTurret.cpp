#include "BaseTurret.h"

#include "Characters/FPSCharacter/SWFPSCharacter.h"


ABaseTurret::ABaseTurret()
{
	PrimaryActorTick.bCanEverTick = false;

	Basement = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Basement"));
	Basement->SetRelativeLocation(FVector(0, 0, 0));
	
	Gun = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Gun"));
	Gun->SetupAttachment(Basement);
	Gun->SetRelativeLocation(FVector(0, 0, 0));
}

void ABaseTurret::BeginPlay()
{
	Super::BeginPlay();

	CurrentRotationAngle = GetActorRotation().Yaw;
	TargetRotationAngle = CurrentRotationAngle + MaxRotationAngle/2.0f;
	
	CollisionParams.AddIgnoredActor(this);
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &ABaseTurret::TurretUpdate, 0.05f, true);
}

void ABaseTurret::TurretUpdate()
{
	if(!Captured)
	{
		const FVector StartLocation = GetActorLocation();
		const FVector Direction = GetActorForwardVector();
		const FVector EndLocation = StartLocation + Direction * RangeOfVisibility;

		DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Blue, false, 0.1f, 0, 0);
		DrawDebugSphere(GetWorld(), EndLocation, SphereRadius, 10, FColor::Red, false, 0.1f);
		DrawDebugSphere(GetWorld(), StartLocation, SphereRadius, 10, FColor::Red, false, 0.1f);

		const FCollisionShape CollisionShape = FCollisionShape::MakeSphere(SphereRadius);
		TArray<FHitResult> HitResults;

		const bool bHit = GetWorld()->SweepMultiByChannel(HitResults, StartLocation, EndLocation,
		                                                  FQuat::Identity, ECC_Pawn, CollisionShape, CollisionParams);
	 
		if (bHit)
		{
			for (const FHitResult& Hit : HitResults)
			{
				DrawDebugSphere(GetWorld(), Hit.Location, 30, 24, FColor::Green,
			false, 0.1f);
								
				if (Hit.GetActor()->IsA(ASWFPSCharacter::StaticClass()))
				{
					CaptureActor = Hit.GetActor();
					Captured = true;
					break;
				}
			}
		}
	}
	RotateTurret();
}

void ABaseTurret::RotateTurret()
{
	if(!Captured)
	{
		CurrentRotationAngle = FMath::FInterpTo(CurrentRotationAngle, TargetRotationAngle, 0.05f, RotationSpeed);
	
		if (FMath::IsNearlyEqual(CurrentRotationAngle, TargetRotationAngle, 1.0f)) 
		{
			MaxRotationAngle *= -1.0f;
			TargetRotationAngle = CurrentRotationAngle + MaxRotationAngle;
		}
	
		FRotator NewRotation = FRotator(0.0f, CurrentRotationAngle, 0.0f);
		SetActorRelativeRotation(NewRotation);
	}
	else
	{
		const FVector StartLocation = GetActorLocation();
		FHitResult HitResult;
		GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, CaptureActor->GetActorLocation(), ECC_Pawn, CollisionParams);

		const AActor* HitActor = HitResult.GetActor();

		if (HitActor)
		if (HitActor->IsA(ASWFPSCharacter::StaticClass()))
		{
			const FVector PlayerLocation = HitActor->GetActorLocation();
			const FVector DirectionToPlayer = (PlayerLocation - GetActorLocation()).GetSafeNormal();
			FRotator NewRotation = DirectionToPlayer.Rotation();
			float ClampedPitch = FMath::Clamp(NewRotation.Pitch, MinPitch, MaxPitch);
			
			FRotator ZRotation(0.0f, NewRotation.Yaw, 0.0f);
			FRotator XRotation(ClampedPitch, 0.0f, 0.0f);
			
			Basement->SetRelativeRotation(ZRotation);
			Gun->SetRelativeRotation(XRotation);
		}
		else 
		{
			Captured = false;
			CurrentRotationAngle = GetActorRotation().Yaw;
			TargetRotationAngle = CurrentRotationAngle + MaxRotationAngle/2.0f;
		}
	}
}
