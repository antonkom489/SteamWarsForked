#include "SWProjectile.h"

#include "GameFramework/Character.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

ASWProjectile::ASWProjectile()
{
	PrimaryActorTick.bCanEverTick = true;

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(FName("ProjectileMovement"));
	ProjectileMovement->ProjectileGravityScale = 0;
	ProjectileMovement->InitialSpeed = 7000.0f;

	bReplicates = true;
	NetUpdateFrequency = 100.0f;

	// Получаем ссылку на игрока
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (PlayerController)
	{
		// Получаем персонажа игрока
		ACharacter* PlayerCharacter = Cast<ACharacter>(PlayerController->GetCharacter());
		if (PlayerCharacter)
		{
			// Получаем позицию игрока
			FVector PlayerLocation = PlayerCharacter->GetActorLocation();

			// Теперь можно использовать PlayerLocation для дальнейших вычислений
			// Например, передать его в метод Initialize
			Initialize(PlayerLocation);
		}
	}
}

void ASWProjectile::Initialize(FVector TargetLocation)
{
	// Получаем текущее местоположение снаряда
	FVector ProjectileLocation = GetActorLocation();

	// Вычисляем направление к цели (позиции игрока)
	FVector Direction = TargetLocation - ProjectileLocation;

	// Нормализуем вектор направления
	Direction.Normalize();

	// Устанавливаем скорость снаряда в направлении игрока
	ProjectileMovement->Velocity = Direction * ProjectileMovement->InitialSpeed;

	// Вычисляем вращение из вектора направления
	FRotator NewRotation = UKismetMathLibrary::MakeRotFromX(Direction);

	// Устанавливаем новое вращение снаряда
	SetActorRotation(NewRotation);
}
