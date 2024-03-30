
#include "Framework/UHealthComponent.h"

UUHealthComponent::UUHealthComponent()
{

	PrimaryComponentTick.bCanEverTick = false;

}

void UUHealthComponent::TakeDamage(float Damage)
{
	CurrentHealth -= Damage;

	UE_LOG(LogTemp, Warning, TEXT("Took %f Damage, %f Health Remaining"), Damage, CurrentHealth);

	if (CurrentHealth <= 0) 
	{
		UE_LOG(LogTemp, Warning, TEXT("Owner Has Died!"));
		OnDead.Broadcast();
	}
}

void UUHealthComponent::Heal(float AmountToHeal)
{
	CurrentHealth = FMath::Clamp(CurrentHealth + FMath::Abs(AmountToHeal), 0, MaxHealth);
}

void UUHealthComponent::InitializeHealthComponent()
{
	CurrentHealth = MaxHealth;
}



