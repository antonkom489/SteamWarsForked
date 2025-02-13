#include "SWPlayerController.h"

#include "Kismet/GameplayStatics.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "SWPlayerState.h"
#include "Actors/Equipment/Weapons/RangeWeaponItem.h"
#include "Characters/CharacterComponents/AbilitySystem/AttributeSet/AmmoAttributeSet.h"
#include "Characters/CharacterComponents/AbilitySystem/AttributeSet/SWAttributeSet.h"
#include "Characters/FPSCharacter/SWFPSCharacter.h"
#include "Characters/InputData/InputDataAsset.h"
#include "UI/SWHUDWidget.h"

ASWPlayerController::ASWPlayerController() :
	RotArrayX { 0.f, 0.f, 0.f },
	RotArrayY { 0.f, 0.f, 0.f }
{
}

void ASWPlayerController::CreateHUD()
{
	// Only create once
	if (UIHUDWidget)
	{
		return;
	}

	if (!UIHUDWidgetClass)
	{
		UE_LOG(LogTemp, Error, TEXT("%s() Missing UIHUDWidgetClass. Please fill in on the Blueprint of the PlayerController."), *FString(__FUNCTION__));
		return;
	}

	// Only create a HUD for local player
	if (!IsLocalPlayerController())
	{
		return;
	}

	// Need a valid PlayerState to get attributes from
	ASWPlayerState* PS = GetPlayerState<ASWPlayerState>();
	if (!PS)
	{
		return;
	}

	UIHUDWidget = CreateWidget<USWHUDWidget>(this, UIHUDWidgetClass);
	UIHUDWidget->AddToViewport();

	// Set attributes
	UIHUDWidget->SetCurrentHealth(PS->GetHealth());
	UIHUDWidget->SetMaxHealth(PS->GetMaxHealth());
	UIHUDWidget->SetHealthPercentage(PS->GetHealth() / PS->GetMaxHealth());
	UIHUDWidget->SetHealthRegenRate(PS->GetHealthRegenRate());
	UIHUDWidget->SetCurrentStamina(PS->GetStamina());
	UIHUDWidget->SetMaxStamina(PS->GetMaxStamina());
	UIHUDWidget->SetStaminaPercentage(PS->GetStamina() / PS->GetMaxStamina());
	UIHUDWidget->SetStaminaRegenRate(PS->GetStaminaRegenRate());
	UIHUDWidget->SetHeroLevel(PS->GetCharacterLevel());

	ASWFPSCharacter* Hero = GetPawn<ASWFPSCharacter>();
	if (Hero)
	{
		ARangeWeaponItem* CurrentWeapon = Hero->GetCurrentWeapon();
		if (CurrentWeapon)
		{
			UIHUDWidget->SetEquippedWeaponSprite(CurrentWeapon->PrimaryIcon);
			UIHUDWidget->SetEquippedWeaponStatusText(CurrentWeapon->GetDefaultStatusText());
			UIHUDWidget->SetPrimaryClipAmmo(Hero->GetPrimaryClipAmmo());
			UIHUDWidget->SetReticle(CurrentWeapon->GetPrimaryHUDReticleClass());

			// PlayerState's Pawn isn't set up yet so we can't just call PS->GetPrimaryReserveAmmo()
			if (PS->GetAmmoAttributeSet())
			{
				FGameplayAttribute Attribute = PS->GetAmmoAttributeSet()->GetReserveAmmoAttributeFromTag(CurrentWeapon->PrimaryAmmoType);
				if (Attribute.IsValid())
				{
					UIHUDWidget->SetPrimaryReserveAmmo(PS->GetAbilitySystemComponent()->GetNumericAttribute(Attribute));
				}
			}
		}
	}
}

USWHUDWidget* ASWPlayerController::GetGSHUD()
{
	return UIHUDWidget;
}

void ASWPlayerController::SetEquippedWeaponPrimaryIconFromSprite(UPaperSprite* InSprite)
{
	if (UIHUDWidget)
	{
		UIHUDWidget->SetEquippedWeaponSprite(InSprite);
	}
}

void ASWPlayerController::SetEquippedWeaponStatusText(const FText& StatusText)
{
	if (UIHUDWidget)
	{
		UIHUDWidget->SetEquippedWeaponStatusText(StatusText);
	}
}

void ASWPlayerController::SetPrimaryClipAmmo(int32 ClipAmmo)
{
	if (UIHUDWidget)
	{
		UIHUDWidget->SetPrimaryClipAmmo(ClipAmmo);
	}
}

void ASWPlayerController::SetPrimaryReserveAmmo(int32 ReserveAmmo)
{
	if (UIHUDWidget)
	{
		UIHUDWidget->SetPrimaryReserveAmmo(ReserveAmmo);
	}
}

void ASWPlayerController::SetSecondaryClipAmmo(int32 SecondaryClipAmmo)
{
	if (UIHUDWidget)
	{
		UIHUDWidget->SetSecondaryClipAmmo(SecondaryClipAmmo);
	}
}

void ASWPlayerController::SetSecondaryReserveAmmo(int32 SecondaryReserveAmmo)
{
	if (UIHUDWidget)
	{
		UIHUDWidget->SetSecondaryReserveAmmo(SecondaryReserveAmmo);
	}
}

void ASWPlayerController::SetHUDReticle(TSubclassOf<class USWHUDReticle> ReticleClass)
{
	// !GetWorld()->bIsTearingDown Stops an error when quitting PIE while targeting when the EndAbility resets the HUD reticle
	if (UIHUDWidget && GetWorld() && !GetWorld()->bIsTearingDown)
	{
		UIHUDWidget->SetReticle(ReticleClass);
	}
}

void ASWPlayerController::ClientSetControlRotation_Implementation(FRotator NewRotation)
{
	SetControlRotation(NewRotation);
}

bool ASWPlayerController::ClientSetControlRotation_Validate(FRotator NewRotation)
{
	return true;
}

void ASWPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	ASWPlayerState* PS = GetPlayerState<ASWPlayerState>();
	if (PS)
	{
		// Init ASC with PS (Owner) and our new Pawn (AvatarActor)
		PS->GetAbilitySystemComponent()->InitAbilityActorInfo(PS, InPawn);
	}
}

void ASWPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	// For edge cases where the PlayerState is repped before the Hero is possessed.
	CreateHUD();
}

void ASWPlayerController::Kill()
{
	ServerKill();
	Restart();
}

void ASWPlayerController::ServerKill_Implementation()
{
	ASWPlayerState* PS = GetPlayerState<ASWPlayerState>();
	if (PS)
	{
		PS->GetAttributeSet()->SetHealth(0.0f);
	}
}

bool ASWPlayerController::ServerKill_Validate()
{
	return true;
}

void ASWPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		// add the mapping context so we get controls
		Subsystem->AddMappingContext(InputMappingContext, 0);

		UE_LOG(LogTemp, Warning, TEXT("BeginPlay"));
	}
}

void ASWPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if(InputActions)
		{
			// Moving
			EnhancedInputComponent->BindAction(InputActions->MoveAction, ETriggerEvent::Triggered, this, &ThisClass::Input_Move);
			//EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &ThisClass::StopMove);

			// Looking
			EnhancedInputComponent->BindAction(InputActions->LookMouseAction, ETriggerEvent::Triggered, this, &ThisClass::Input_LookMouse);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ASWPlayerController::Input_Move(const FInputActionValue& InputActionValue)
{
	ASWFPSCharacter* HeroCharacter = GetPawn<ASWFPSCharacter>();
	AController* Controller = HeroCharacter ? HeroCharacter->GetController() : nullptr;

	if (Controller && HeroCharacter->IsAlive())
	{
		const FVector2D Value = InputActionValue.Get<FVector2D>();
		const FRotator MovementRotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);

		if (Value.X != 0.0f)
		{
			const FVector MovementDirection = MovementRotation.RotateVector(FVector::RightVector);
			HeroCharacter->AddMovementInput(MovementDirection, Value.X);
		}

		if (Value.Y != 0.0f)
		{
			const FVector MovementDirection = MovementRotation.RotateVector(FVector::ForwardVector);
			HeroCharacter->AddMovementInput(MovementDirection, Value.Y);
		}
	}
}

void ASWPlayerController::Input_LookMouse(const FInputActionValue& InputActionValue)
{
	ASWFPSCharacter* HeroCharacter = GetPawn<ASWFPSCharacter>();

	if (!HeroCharacter)
	{
		return;
	}
	float LookScaleModifier = 1.f;
	LookScaleModifier *= FMath::Lerp(1.f, ADSSensitivityScale, HeroCharacter->GetADSAlpha());
	const FVector2D Value = InputActionValue.Get<FVector2D>() * LookScaleModifier;

	RotArrayX[RotCacheIndex] = Value.X;
	float result = 0.f;
	for (int i = 0; i < MaxRotCache; i++)
	{
		result += RotArrayX[i];
	}
	HeroCharacter->AddControllerYawInput(result / MaxRotCache);
	

	RotArrayY[RotCacheIndex] = Value.Y;
	result = 0.f;
	for (int i = 0; i < MaxRotCache; i++)
	{
		result += RotArrayY[i];
	}
	HeroCharacter->AddControllerPitchInput(result / MaxRotCache);

	RotCacheIndex++;
	RotCacheIndex %= MaxRotCache;

	ARangeWeaponItem* currentWeapon = HeroCharacter->GetCurrentWeapon();
	if (currentWeapon && currentWeapon->bIsRecoilPitchRecoveryActive)
	{
		FRotator currentRotation = GetControlRotation();
		FRotator checkpointRotation = currentWeapon->RecoilCheckpoint;

		FRotator deltaRot = (currentRotation - checkpointRotation).GetNormalized();

		if (Value.Y < 0.f)
		{
			currentWeapon->bIsRecoilPitchRecoveryActive = false;
			currentWeapon->bIsRecoilNeutral = true;
			return;
		}

		if (deltaRot.Pitch < 0.f)
		{
			currentWeapon->bUpdateRecoilPitchCheckpointInNextShot = true;
		}

		if (Value.X != 0.f)
		{
			if (currentWeapon->bIsRecoilYawRecoveryActive)
			{
				currentWeapon->bIsRecoilYawRecoveryActive = false;
			}

			currentWeapon->bUpdateRecoilYawCheckpointInNextShot = true;
		}

	}
}

void ASWPlayerController::ShowPauseMenu()
{
	if (SWPauseMenuWidgetClass)
	{
		PauseMenu = CreateWidget<UUserWidget>(this, SWPauseMenuWidgetClass);
		if (PauseMenu)
		{
			PauseMenu->AddToViewport();
			bIsPaused = true;
			SetInputMode(FInputModeUIOnly());
			bShowMouseCursor = true; 
		}
	}
}

void ASWPlayerController::HidePauseMenu()
{
	if (PauseMenu)
	{
		PauseMenu->RemoveFromParent();
	}
    
	bIsPaused = false;
	SetInputMode(FInputModeGameOnly());
	bShowMouseCursor = false; 
}

void ASWPlayerController::TogglePauseMenu()
{
	if (UGameplayStatics::IsGamePaused(GetWorld()))
	{
		UGameplayStatics::SetGamePaused(GetWorld(), false);
	}
	else
	{
		UGameplayStatics::SetGamePaused(GetWorld(), true);
	}
}

void ASWPlayerController::Restart()
{
	UWorld* World = GetWorld();
	if (World)
	{
		FName CurrentLevelName = FName(*World->GetMapName());
        
		UGameplayStatics::OpenLevel(World, CurrentLevelName);
	}
}
