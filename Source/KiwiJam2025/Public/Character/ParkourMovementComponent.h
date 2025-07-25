// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Character/ClimbableDetectorComponent.h"

#include "ParkourMovementComponent.generated.h"

UENUM()
enum class EClimbPhase : uint8
{
    None,
    Approach,
    Grab,
    PullUp
};

/**
 * Movement comp with added parkor stuff
 */
UCLASS()
class KIWIJAM2025_API UParkourMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
    // Custom movement modes
    enum ECustomMovementMode
    {
        MOVE_Climb = MOVE_Custom + 0,
        MOVE_WallRun = MOVE_Custom + 1,
        MOVE_Vault = MOVE_Custom + 2,
        // etc.
    };

    UParkourMovementComponent();

    virtual void PhysCustom(float deltaTime, int32 Iterations) override;

    void BeginClimb(const FClimbableSurfaceResult& Surface);

protected:
    void PhysClimb(float deltaTime, int32 Iterations);
    void PhysWallRun(float deltaTime, int32 Iterations);
    void PhysVault(float deltaTime, int32 Iterations);


private:

    UPROPERTY(EditAnywhere, Category = "Debug")
    bool bDebugDraw = true;

    FVector ClimbStartLocation;
    FVector ClimbMidLocation;     // ledge grab point
    FVector ClimbTargetLocation;  // over-the-top location
    FRotator DesiredClimbFacingRotation;

    float ClimbDuration = 0.5f;
    float ClimbElapsed = 0.f;
    bool bClimbActive = false;

    float ClimbStartPitch = 0.f;
    float ClimbTargetPitch = 45.f; // Look upward slightly

    EClimbPhase ClimbPhase = EClimbPhase::None;

    // Curve for climb progress
    UPROPERTY(EditAnywhere, Category = "Parkour|Climb") 
    UCurveFloat* ClimbProgressCurve = nullptr; 


    UPROPERTY(EditAnywhere, Category = "Parkour|Climb")
    float ApproachTime = 0.25f;

    UPROPERTY(EditAnywhere, Category = "Parkour|Climb")
    float GrabTime = 0.25f;

    UPROPERTY(EditAnywhere, Category = "Parkour|Climb")
    float PullUpTime = 0.5f;
};
