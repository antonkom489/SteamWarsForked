#include "BaseTurret.h"
#include "Characters/FPSCharacter/SWFPSCharacter.h"
#include "Perception/PawnSensingComponent.h"
#include "Engine/Engine.h"

ABaseTurret::ABaseTurret()
{
	PrimaryActorTick.bCanEverTick = false;
	ArrowComponent = CreateDefaultSubobject<UArrowComponent>(TEXT("ArrowComponent"));
	ArrowComponent->SetupAttachment(Gun);
	ArrowComponent->SetRelativeLocation(FVector(0, 0, 0));

	Basement = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Basement"));
	Basement->SetRelativeLocation(FVector(0, 0, 0));

	Gun = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Gun"));
	Gun->SetupAttachment(Basement);
	Gun->SetRelativeLocation(FVector(0, 0, 0));

	PawnSensingComp = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensingComp"));

	if (!PawnSensingComp)
	{
		UE_LOG(LogTemp, Error, TEXT("PawnSensingComponent не инициализирован!"));
	}

	PawnSensingComp->SightRadius = 50000.f;
	PawnSensingComp->SensingInterval = .25f; // 4 times per second
	PawnSensingComp->SetPeripheralVisionAngle(90.f);
}

void ABaseTurret::OnSeePawn(APawn* SeenPawn)
{
	FString message = TEXT("Saw Actor ") + SeenPawn->GetName();
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, message);
	if (SeenPawn && SeenPawn->ActorHasTag("Player"))
	{
		CaptureActor = SeenPawn;
		turretState = ETurretState::Fire;

		UE_LOG(LogTemp, Warning, TEXT("Work"));
	}
}

void ABaseTurret::BeginPlay()
{
	Super::BeginPlay();
	turretState = ETurretState::Looking;

	if (PawnSensingComp)
	{
		PawnSensingComp->OnSeePawn.AddDynamic(this, &ABaseTurret::OnSeePawn);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("PawnSensingComponent не инициализирован!"));
	}

	if (PawnSensingComp->OnSeePawn.IsBound())
	{
		UE_LOG(LogTemp, Warning, TEXT("На событие OnSeePawn есть подписка."));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("На событие OnSeePawn нет подписки."));
	}

	CurrentRotationAngle = GetActorRotation().Yaw;
	TargetRotationAngle = CurrentRotationAngle + MaxRotationAngle / 2.0f;

	CollisionParams.AddIgnoredActor(this);
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &ABaseTurret::RotateTurret, 0.05f, true);
}

void ABaseTurret::RotateTurret()
{
	switch (turretState)
	{
	case ETurretState::Looking:
		LookMode();
		EndShoot();
		break;

	case ETurretState::Fire:
		AttackMode();
		break;
	}
}

void ABaseTurret::AttackMode()
{
	const FVector StartLocation = GetActorLocation();
	FHitResult HitResult;
	GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, CaptureActor->GetActorLocation(),
		ECC_Pawn, CollisionParams);

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
			StartShoot();
		}
		else
		{
			turretState = ETurretState::Looking;
			CurrentRotationAngle = GetActorRotation().Yaw;
			TargetRotationAngle = CurrentRotationAngle + MaxRotationAngle / 2.0f;
		}
}

void ABaseTurret::LookMode()
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

void ABaseTurret::Shoot()
{
	UE_LOG(LogTemp, Warning, TEXT("Выстрел!"));
}

void ABaseTurret::StartShoot()
{
	GetWorld()->GetTimerManager().SetTimer(ShootTimerHandle, this, &ABaseTurret::Shoot, ShootDelay, true);
}

void ABaseTurret::EndShoot()
{
	GetWorld()->GetTimerManager().ClearTimer(ShootTimerHandle);
}