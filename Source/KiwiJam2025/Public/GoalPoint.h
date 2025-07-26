// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GoalPoint.generated.h"

class USphereComponent;
class UBillboardComponent;
class AParkourCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGoalReached, AActor*, GoalActor);

UCLASS()
class KIWIJAM2025_API AGoalPoint : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGoalPoint();

	// Event that other classes (GameMode, Player) can bind to
	UPROPERTY(BlueprintAssignable, Category = "Goal")
	FOnGoalReached OnGoalReached;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Trigger overlap
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
		const FHitResult& SweepResult);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsActive = true;

	UPROPERTY(EditAnywhere, Category = "Goal")
	TSubclassOf<UUserWidget> GoalMarkerClass; // Widget class for marker

private:
	UPROPERTY(VisibleAnywhere, Category = "Components")
	USphereComponent* CollisionSphere;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UBillboardComponent* IconBillboard; // For editor visualization

	// Store the marker widget
	UPROPERTY()
	UUserWidget* GoalMarkerWidget;

	AParkourCharacter* PlayerChar;

	bool bMarkerAdded = false;

public:	
	void Tick(float DeltaTime) override;

	// Returns location for map marker
	FVector GetGoalLocation() const;
};
