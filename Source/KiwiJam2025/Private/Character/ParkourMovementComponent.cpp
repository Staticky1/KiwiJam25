// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/ParkourMovementComponent.h"
#include "Character/ClimbableDetectorComponent.h"
#include "GameFramework/Character.h" 
#include "Components/CapsuleComponent.h"
#include "Curves/CurveVector.h"
#include "Camera/CameraComponent.h"
#include "Character/ParkourCharacter.h"

UParkourMovementComponent::UParkourMovementComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UParkourMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bShouldApplyPostVaultVelocity && MovementMode == MOVE_Walking)
    {
        Launch(PendingPostVaultVelocity);
        bShouldApplyPostVaultVelocity = false;
    }

    if (VaultCameraTiltCurve && bVaulting) 
    {
        float Alpha = FMath::Clamp(VaultElapsed / VaultTime, 0.f, 1.f);
        float CurveValue = VaultCameraTiltCurve->GetFloatValue(Alpha);
        CurrentVaultTilt = CurveValue * MaxCameraTilt;
    }
    else
    {
        CurrentVaultTilt = FMath::FInterpTo(CurrentVaultTilt, 0.f, DeltaTime, 6.f); // Smooth reset
    }

    if (AParkourCharacter* Character = Cast<AParkourCharacter>(GetOwner()))  
    {
        FRotator TiltedRotation{};
        TiltedRotation.Roll += CurrentVaultTilt;  
        

        Character->AddCameraRotation(TiltedRotation); 
    }
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

void UParkourMovementComponent::BeginVault(const FClimbableSurfaceResult& Surface)
{
    if (!VaultCurve || !CharacterOwner) return;

    VaultStart = CharacterOwner->GetActorLocation();
    VaultTarget = Surface.ImpactPoint + (Surface.ImpactNormal * 100.f);
    VaultHeight = Surface.SurfaceHeight;
    // Direction for vault curve X axis
    VaultDirection = Surface.SurfaceForward;

    VaultMomentumVelocity = Velocity;

    VaultElapsed = 0.f;
    bVaulting = true;

    SetMovementMode(MOVE_Custom, MOVE_Vault); // 1 = Vault
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

            FRotator DesiredRotation = Current;
            DesiredRotation.Pitch = ClimbTargetPitch;
            FRotator NewPitch = FMath::RInterpTo(Current, DesiredRotation, deltaTime, 6.f);
            Current = NewPitch;

            Controller->SetControlRotation(Current);
        }
    

    }

    if (ClimbPhase == EClimbPhase::Grab)
    {
        if (AController* Controller = CharacterOwner->GetController())
        {
            FRotator Current = Controller->GetControlRotation();
            FRotator DesiredRotation = Current;
            DesiredRotation.Pitch = -1.0f * (ClimbTargetPitch * 0.5f);
            FRotator NewPitch = FMath::RInterpTo(Controller->GetControlRotation(), DesiredRotation, deltaTime, 4.f);
            Current = NewPitch;

            Controller->SetControlRotation(Current); 
        }
    }

    if (CharacterOwner->GetCapsuleComponent())
    {
        CharacterOwner->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    float Alpha = FMath::Clamp(ClimbElapsed / CurrentPhaseDuration, 0.f, 1.f);
    float CurveAlpha = ClimbProgressCurve ? ClimbProgressCurve->GetFloatValue(Alpha) : Alpha;
    FVector NewLocation = FMath::Lerp(PhaseStart, PhaseEnd, CurveAlpha);


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
    if (!VaultCurve || !CharacterOwner || !bVaulting)
    {
        SetMovementMode(MOVE_Walking);
        return;
    }

    VaultElapsed += deltaTime;
    float Time = FMath::Clamp(VaultElapsed / VaultTime, 0.f, 1.f);;
    if (CharacterOwner->GetCapsuleComponent())
    {
        CharacterOwner->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }


    if (Time >= 1.0f)
    {
        // End vault
          // Estimate forward velocity from final curve direction
        float CurveDelta = VaultForwardDistance;
        FVector ForwardWorldDelta = VaultDirection * CurveDelta;

        // Capture velocity to apply next frame
        PendingPostVaultVelocity = ForwardWorldDelta + VaultMomentum + VaultMomentumVelocity;
        bShouldApplyPostVaultVelocity = true;

        bVaulting = false;
        SetMovementMode(MOVE_Walking);
        if (CharacterOwner->GetCapsuleComponent())
        {
            CharacterOwner->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        }

        return;
    }

    // Get relative vault offset from curve
    FVector LocalOffset = VaultCurve->GetVectorValue(Time);

    // Build world-space offset using vault direction as X
    FVector Right = FVector::CrossProduct(FVector::UpVector, VaultDirection);
    FVector Up = FVector::UpVector;

    FVector WorldOffset =
        VaultDirection * (LocalOffset.X * VaultForwardDistance) +
        Right * LocalOffset.Y +
        Up * (LocalOffset.Z * VaultHeight + 50.f);

    FVector NewLocation = VaultStart + WorldOffset;

    // Move character
    FHitResult Hit;
    SafeMoveUpdatedComponent(NewLocation - CharacterOwner->GetActorLocation(), CharacterOwner->GetActorRotation(), true, Hit);

}
