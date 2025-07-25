// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/ClimbableDetectorComponent.h"
#include "GameFramework/Character.h" 

// Sets default values for this component's properties
UClimbableDetectorComponent::UClimbableDetectorComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}

void UClimbableDetectorComponent::SetOwnerCharacter(ACharacter* Character)
{
    if (Character)
    {
        OwnerCharacter = Character;
    }
}

bool UClimbableDetectorComponent::DetectClimbableSurface(FClimbableSurfaceResult& OutResult)
{
    if (!OwnerCharacter) return false;


    FHitResult ForwardHit;
    if (!TraceForward(ForwardHit))
    {
        // Optional debug
        if (bDebugDraw)
        {
            FVector Start = OwnerCharacter->GetActorLocation() + FVector(0, 0, VerticalTraceHeight * 0.5f);
            FVector End = Start + OwnerCharacter->GetActorForwardVector() * ForwardTraceDistance;
            DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 1.0f, 0, 2.f);
        }
        return false;
    }

    FVector LedgeTopLocation;
    if (!TraceLedgeTop(ForwardHit.ImpactPoint - (ForwardHit.ImpactNormal * 20), LedgeTopLocation))
    {
        if (bDebugDraw)
        {
            DrawDebugPoint(GetWorld(), ForwardHit.ImpactPoint, 10.f, FColor::Orange, false, 1.f);
            DrawDebugPoint(GetWorld(), LedgeTopLocation, 10.f, FColor::Orange, false, 1.f);
        }
        return false;
    }

    const float SurfaceHeight = LedgeTopLocation.Z - OwnerCharacter->GetActorLocation().Z;

    if (SurfaceHeight < MinLedgeHeight || SurfaceHeight > MaxLedgeHeight)
        return false;

    OutResult.bIsValid = true;
    OutResult.ImpactPoint = LedgeTopLocation;
    OutResult.ImpactNormal = ForwardHit.ImpactNormal;
    OutResult.SurfaceForward = -ForwardHit.ImpactNormal;
    OutResult.HitActor = ForwardHit.GetActor();
    OutResult.SurfaceHeight = SurfaceHeight;
    OutResult.SurfaceType = EClimbableSurfaceType::Ledge; // We'll classify more later

    // Debug visualization 
    if (bDebugDraw) 
    {
        // Forward Trace
        FVector Start = OwnerCharacter->GetActorLocation() + FVector(0, 0, VerticalTraceHeight * 0.5f);
        FVector End = Start + OwnerCharacter->GetActorForwardVector() * ForwardTraceDistance;
        DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, 2.f, 0, 2.f);

        // Impact normal
        FVector NormalEnd = ForwardHit.ImpactPoint + ForwardHit.ImpactNormal * 50.f;
        DrawDebugLine(GetWorld(), ForwardHit.ImpactPoint, NormalEnd, FColor::Blue, false, 2.f, 0, 2.f);

        // Surface/ledge location
        DrawDebugSphere(GetWorld(), LedgeTopLocation, 15.f, 12, FColor::Cyan, false, 2.f);

        // Surface height value (optional as text)
        DrawDebugString(GetWorld(), LedgeTopLocation + FVector(0, 0, 20.f),
            FString::Printf(TEXT("Height: %.1f"), SurfaceHeight),
            nullptr, FColor::White, 2.f, false);
    }

    return true;
}


// Called when the game starts
void UClimbableDetectorComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

bool UClimbableDetectorComponent::TraceForward(FHitResult& OutHit)
{
    FVector Start = OwnerCharacter->GetActorLocation() + FVector(0, 0, VerticalTraceHeight * 0.5f);
    FVector End = Start + OwnerCharacter->GetActorForwardVector() * ForwardTraceDistance;

    if (bDebugDraw)
    {
        DrawDebugLine(GetWorld(), Start, End, FColor::Orange, false, 2.f, 0, 2.f); 
    }

    FCollisionQueryParams Params; 
    Params.AddIgnoredActor(OwnerCharacter); 

    return GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, TraceChannel, Params); 
}

bool UClimbableDetectorComponent::TraceLedgeTop(const FVector& ForwardHitLocation, FVector& OutLedgeLocation)
{
    FVector Start = ForwardHitLocation + FVector(0, 0, MaxLedgeHeight);
    FVector End = ForwardHitLocation + FVector(0, 0, MinLedgeHeight);

    FHitResult LedgeHit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(OwnerCharacter);

    if (bDebugDraw)
    {
        DrawDebugLine(GetWorld(), Start, End, FColor::Orange, false, 2.f, 0, 2.f);
    }

    if (!GetWorld()->LineTraceSingleByChannel(LedgeHit, Start, End, TraceChannel, Params))
        return false;

    OutLedgeLocation = LedgeHit.ImpactPoint;
    return true;
}

void UClimbableDetectorComponent::DrawDebugBoxAtPoint(UWorld* World, const FVector& Point, const FColor& Color, float Size)
{
    DrawDebugBox(World, Point, FVector(Size), Color, false, 2.f, 0, 1.f); 
}

