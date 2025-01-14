#include "SWCharacterEquipmentComponent.h"
#include "Actors/Equipment/Weapons/RangeWeaponItem.h"
#include "Characters/SWBaseCharacter.h"
#include "Characters/FPSCharacter/SWFPSCharacter.h"


EEquipableItemType USWCharacterEquipmentComponent::GetCurrentEquippedItemType() const
{
	EEquipableItemType Result = EEquipableItemType::None;
	
	if(IsValid(CurrentEquippedWeapon))
	{
		Result = CurrentEquippedWeapon->GetItemType();
	}
	
	return Result;
}

void USWCharacterEquipmentComponent::FireShoot()
{
	if(IsValid(CurrentEquippedWeapon))
	{
		CurrentEquippedWeapon->FireShoot();
	}
}

void USWCharacterEquipmentComponent::BeginPlay()
{
	Super::BeginPlay();
	checkf(GetOwner()->IsA<ASWBaseCharacter>(),
		TEXT("USWCharacterEquipmentComponent::BeginPlay() "
	   "CharacterEquipmentComponent can be used only with ASWBaseCharacter"));
	CachedBaseCharacter = StaticCast<ASWBaseCharacter*>(GetOwner());
	CreateLoadout();
}

void USWCharacterEquipmentComponent::CreateLoadout()
{
	if(!IsValid(SideArmClass)) return;
	
	CurrentEquippedWeapon = GetWorld()->SpawnActor<ARangeWeaponItem>(SideArmClass);
	
	if(CachedBaseCharacter->IsA<ASWFPSCharacter>())
	{
		ASWFPSCharacter* FPSCharacter = Cast<ASWFPSCharacter>(CachedBaseCharacter);
		CurrentEquippedWeapon->AttachToComponent(FPSCharacter->GetFPSMesh(), FAttachmentTransformRules::KeepRelativeTransform, SocketCharacterWeapon);
	}
	else
	{
		CurrentEquippedWeapon->AttachToComponent(CachedBaseCharacter->GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, SocketCharacterWeapon);
	}

	CurrentEquippedWeapon->SetOwner(CachedBaseCharacter.Get());
}
