#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SWPlayerController.generated.h"

class UPaperSprite;
struct FInputActionValue;
class UInputMappingContext;

UCLASS()
class STEAMWARS_API ASWPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ASWPlayerController();

	void CreateHUD();
	//void CreatePauseHUD();

	class USWHUDWidget* GetGSHUD();


	/**
	* Weapon HUD info
	*/

	UFUNCTION(BlueprintCallable, Category = "GAS|UI")
	void SetEquippedWeaponPrimaryIconFromSprite(UPaperSprite* InSprite);

	UFUNCTION(BlueprintCallable, Category = "GAS|UI")
	void SetEquippedWeaponStatusText(const FText& StatusText);

	UFUNCTION(BlueprintCallable, Category = "GAS|UI")
	void SetPrimaryClipAmmo(int32 ClipAmmo);

	UFUNCTION(BlueprintCallable, Category = "GAS|UI")
	void SetPrimaryReserveAmmo(int32 ReserveAmmo);

	UFUNCTION(BlueprintCallable, Category = "GAS|UI")
	void SetSecondaryClipAmmo(int32 SecondaryClipAmmo);

	UFUNCTION(BlueprintCallable, Category = "GAS|UI")
	void SetSecondaryReserveAmmo(int32 SecondaryReserveAmmo);

	UFUNCTION(BlueprintCallable, Category = "GAS|UI")
	void SetHUDReticle(TSubclassOf<class USWHUDReticle> ReticleClass);

	UFUNCTION(Client, Reliable, WithValidation)
	void ClientSetControlRotation(FRotator NewRotation);
	void ClientSetControlRotation_Implementation(FRotator NewRotation);
	bool ClientSetControlRotation_Validate(FRotator NewRotation);

	UFUNCTION(BlueprintCallable, Category = "GAS|UI")
	void TogglePauseMenu();
	
	UFUNCTION(BlueprintCallable, Category = "GAS|Gameplay")
	void Restart();

protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GAS|UI")
	TSubclassOf<class USWHUDWidget> UIHUDWidgetClass;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GAS|UI")
	TSubclassOf<UUserWidget> SWPauseMenuWidgetClass;

	UPROPERTY(BlueprintReadWrite, Category = "GAS|UI")
	class USWHUDWidget* UIHUDWidget;



	// Server only
	virtual void OnPossess(APawn* InPawn) override;

	virtual void OnRep_PlayerState() override;

	UFUNCTION(Exec)
	void Kill();

	UFUNCTION(Server, Reliable)
	void ServerKill();
	void ServerKill_Implementation();
	bool ServerKill_Validate();

	bool bIsPaused;

	void ShowPauseMenu();
	void HidePauseMenu();
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputMappingContext* InputMappingContext = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	float ADSSensitivityScale{ 0.3f };

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputDataAsset* InputActions;

	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	int64 MaxRotCache = 3;
	float RotArrayX[3];
	float RotArrayY[3];
	int64 RotCacheIndex = 0;

	UUserWidget* PauseMenu;
	
	void Input_Move(const FInputActionValue& InputActionValue);
	void Input_LookMouse(const FInputActionValue& InputActionValue);
};
