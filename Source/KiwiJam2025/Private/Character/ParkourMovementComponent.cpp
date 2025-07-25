// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/ParkourMovementComponent.h"
#include "Character/ClimbableDetectorComponent.h"
#include "GameFramework/Character.h" 
#include "Components/CapsuleComponent.h"

UParkourMovementComponent::UParkourMovementComponent()
{

}

void UParkourMovementComponent::PhysCustom(float deltaTime, int32 Iterations)
{
    switch (CustomMovementMode)
    {
    case MOVE_Climb:
        PhysClimb(deltaTime, Iterations);
        break;
    case MOVE_WallRun:
        PhysWallRun(deltaTime, Iterations);
        break;
    case MOVE_Vault:
        PhysVault(deltaTime, Iterations);
        break;
    default:
        Super::PhysCustom(deltaTime, Iterations); 
        break;
    }
}

void UParkourMovementComponent::BeginClimb(const FClimbableSurfaceResult& Surface)
{
    if (!CharacterOwner) return;


    ClimbPhase = EClimbPhase::Approach;
    ClimbElapsed = 0.f;
    bClimbActive = true;

    // Start = current position
    ClimbStartLocation = CharacterOwner->GetActorLocation();

    ClimbMidLocation = Surface.ImpactPoint + Surface.SurfaceForward * -50.f + FVector(0.f, 0.f, -40.f); // adjust Z for ledge grab
    // End = up and over
    ClimbTargetLocation = Surface.ImpactPoint + Surface.SurfaceForward * 30.f + FVector(0.f, 0.f, 120.f);

    SetMovementMode(MOVE_Custom, MOVE_Climb);

    // Face the ledge
    DesiredClimbFacingRotation = Surface.SurfaceForward.Rotation();
    DesiredClimbFacingRotation.Pitch = 0.f;
    DesiredClimbFacingRotation.Roll = 0.f;

    if (bDebugDraw)
    {
        DrawDebugSphere(GetWorld(), ClimbStartLocation, 8.f, 8, FColor::Blue, false, 5.f);
        DrawDebugSphere(GetWorld(), ClimbMidLocation, 8.f, 8, FColor::Yellow, false, 5.f);
        DrawDebugSphere(GetWorld(), ClimbTargetLocation, 8.f, 8, FColor::Green, false, 5.f);
    }

}

void UParkourMovementComponent::PhysClimb(float deltaTime, int32 Iterations)
{
    if (!bClimbActive || !CharacterOwner) return;

    ClimbElapsed += deltaTime; 

    float CurrentPhaseDuration = 0.f; 
    FVector PhaseStart, PhaseEnd; 

    switch (ClimbPhase)
    {
    case EClimbPhase::Approach:
        CurrentPhaseDuration = ApproachTime;
        PhaseStart = ClimbStartLocation;
        PhaseEnd = ClimbMidLocation;
        break;
    case EClimbPhase::Grab:
        CurrentPhaseDuration = GrabTime;
        PhaseStart = ClimbMidLocation;
        PhaseEnd = FVector(ClimbMidLocation.X, ClimbMidLocation.Y, ClimbTargetLocation.Z ); // ledge top hold
        break;
    case EClimbPhase::PullUp:
        CurrentPhaseDuration = PullUpTime;
        PhaseStart = FVector(ClimbMidLocation.X, ClimbMidLocation.Y, ClimbTargetLocation.Z);
        PhaseEnd = ClimbTargetLocation;
        break;
    default:
        return;
    }

    if (ClimbPhase == EClimbPhase::Approach )
    {
        if (AController* Controller = CharacterOwner->GetController())
        {
            FRotator Current = Controller->GetControlRotation();
            FRotator Smoothed = FMath::RInterpTo(Current, DesiredClimbFacingRotation, deltaTime, 8.f);
            Controller->SetControlRotation(Smoothed);
        }
    

    }

    if (CharacterOwner->GetCapsuleComponent())
    {
        CharacterOwner->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    float Alpha = FMath::Clamp(ClimbElapsed / CurrentPhaseDuration, 0.f, 1.f);
    float CurveAlpha = ClimbProgressCurve ? ClimbProgressCurve->GetFloatValue(Alpha) : Alpha;
    FVector NewLocation = FMath::Lerp(PhaseStart, PhaseEnd, CurveAlpha);

    if (ClimbPhase == EClimbPhase::Grab)
    {
        // Interpolate camera pitch
        if (AController* Controller = CharacterOwner->GetController())
        {
            FRotator ControlRot = Controller->GetControlRotation();
            float NewPitch = FMath::Lerp(ClimbStartPitch, ClimbTargetPitch, CurveAlpha);
            ControlRot.Pitch = NewPitch;

            Controller->SetControlRotation(ControlRot);
        }
    }

    FVector DeltaMove = NewLocation - CharacterOwner->GetActorLocation();
    FHitResult Hit;
    SafeMoveUpdatedComponent(DeltaMove, CharacterOwner->GetActorRotation(), true, Hit);

    if (Alpha >= 1.f)
    {
        ClimbElapsed = 0.f;

        switch (ClimbPhase)
        {
        case EClimbPhase::Approach:
            ClimbPhase = EClimbPhase::Grab;
            if (AController* Controller = CharacterOwner->GetController())
            {
                FRotator CurrentRot = Controller->GetControlRotation();
                ClimbStartPitch = CurrentRot.Pitch;

                // Slight upward look — adjust to your taste
                ClimbTargetPitch = -15.f;
            }
            break;
        case EClimbPhase::Grab:
            ClimbPhase = EClimbPhase::PullUp;
            break;
        case EClimbPhase::PullUp:
            bClimbActive = false;
            ClimbPhase = EClimbPhase::None;
            SetMovementMode(MOVE_Walking);
            if (CharacterOwner->GetCapsuleComponent())
            {
                CharacterOwner->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            }
            break;
        default:
            break;
        }
    }

}

void UParkourMovementComponent::PhysWallRun(float deltaTime, int32 Iterations)
{
}

void UParkourMovementComponent::PhysVault(float deltaTime, int32 Iterations)
{
}
