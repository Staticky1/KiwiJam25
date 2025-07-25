// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ClimbableDetectorComponent.generated.h"

UENUM(BlueprintType)
enum class EClimbableSurfaceType : uint8
{
	None,
	Vaultable,
	Ledge,
	Wall,
	Climbable,
	WallRunLeft,
	WallRunRight
};

USTRUCT(BlueprintType)
struct FClimbableSurfaceResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	bool bIsValid = false;

	UPROPERTY(BlueprintReadOnly)
	FVector ImpactPoint;

	UPROPERTY(BlueprintReadOnly)
	FVector ImpactNormal;

	UPROPERTY(BlueprintReadOnly)
	FVector SurfaceForward;

	UPROPERTY(BlueprintReadOnly)
	EClimbableSurfaceType SurfaceType = EClimbableSurfaceType::None;

	UPROPERTY(BlueprintReadOnly)
	float SurfaceHeight = 0.f;

	UPROPERTY(BlueprintReadOnly)
	AActor* HitActor = nullptr;
};

//class ACharacter;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class KIWIJAM2025_API UClimbableDetectorComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UClimbableDetectorComponent();

	void SetOwnerCharacter(ACharacter* Character);

	bool DetectClimbableSurface(FClimbableSurfaceResult& OutResult);

protected:
	UPROPERTY(EditAnywhere)
	float ForwardTraceDistance = 150.f;

	UPROPERTY(EditAnywhere)
	float VerticalTraceHeight = 100.f;

	UPROPERTY(EditAnywhere)
	float MinLedgeHeight = 40.f;

	UPROPERTY(EditAnywhere)
	float MaxLedgeHeight = 140.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bDebugDraw = true;

	UPROPERTY(EditAnywhere)
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;

	TObjectPtr<ACharacter> OwnerCharacter;

public:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	bool TraceForward(FHitResult& OutHit);
	bool TraceLedgeTop(const FVector& ForwardHitLocation, FVector& OutLedgeLocation);

	void DrawDebugBoxAtPoint(UWorld* World, const FVector& Point, const FColor& Color, float Size = 10.f);

};
