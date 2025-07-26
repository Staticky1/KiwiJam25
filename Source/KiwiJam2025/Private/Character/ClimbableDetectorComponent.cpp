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

    //check head space
    FHitResult HeadHit;
    if (TraceHead(HeadHit))
    {
        // Optional debug
        if (bDebugDraw)
        {
            FVector Start = OwnerCharacter->GetActorLocation() + FVector(0, 0, VerticalTraceHeight * 0.5f);
            FVector End = Start + OwnerCharacter->GetActorUpVector() * UpTraceHeight;
            DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 1.0f, 0, 2.f);
        }
        return false;
    }

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
    OutResult.SurfaceType = EClimbableSurfaceType::Ledge; // classify more later

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

bool UClimbableDetectorComponent::CheckVaultSurface(FClimbableSurfaceResult& OutInfo)
{
    if (!OwnerCharacter) return false;

    FVector Start = OwnerCharacter->GetActorLocation();
    FVector Forward = OwnerCharacter->GetActorForwardVector();
    FVector End = Start + Forward * VaultForwardTraceDistance; // Short forward check

    // 1. Forward trace to detect obstacle
    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(OwnerCharacter);
    if (bDebugDraw)
        DrawDebugLine(GetWorld(), Start, End, FColor::Yellow, false, 2.0f); 
    if (!GetWorld()->LineTraceSingleByChannel(Hit, Start, End, TraceChannel, Params))
    {
        return false;
    }

    if (bDebugDraw)
    DrawDebugSphere(GetWorld(), Hit.ImpactPoint, 15.f, 12, FColor::Cyan, false, 2.f);

    float ObstacleTopZ = Hit.ImpactPoint.Z + Hit.Component->Bounds.BoxExtent.Z;
    float PlayerFeetZ = OwnerCharacter->GetActorLocation().Z;

    float ObstacleHeight = ObstacleTopZ - PlayerFeetZ;

    // 2. Height check
    if (ObstacleHeight < VaultObstacleHeightMin || ObstacleHeight > VaultObstacleHeightMax)
        return false;

    // 3. Check for landing spot beyond the obstacle
    FVector VaultCheckStart = Hit.ImpactPoint + Forward * VaultObstacleDistance + FVector(0,0,50);
    FVector VaultCheckEnd = VaultCheckStart - FVector(0, 0, 120);
    if (bDebugDraw)
        DrawDebugLine(GetWorld(), VaultCheckStart, VaultCheckEnd, FColor::Yellow, false, 2.0f);
    FHitResult VaultLandingHit;
    if (GetWorld()->LineTraceSingleByChannel(VaultLandingHit, VaultCheckStart, VaultCheckEnd, TraceChannel, Params))
    {
        if (bDebugDraw)
            DrawDebugSphere(GetWorld(), VaultLandingHit.ImpactPoint, 15.f, 12, FColor::Cyan, false, 5.f);
        return false;
    }

    OutInfo.bIsValid = true;
    OutInfo.ImpactPoint = Hit.ImpactPoint;
    OutInfo.ImpactNormal = Hit.ImpactNormal;
    OutInfo.SurfaceForward = -Hit.ImpactNormal;
    OutInfo.HitActor = Hit.GetActor();
    OutInfo.SurfaceHeight = ObstacleHeight;
    OutInfo.SurfaceType = EClimbableSurfaceType::Vaultable; // We'll classify more later

    if (bDebugDraw)
    {
        DrawDebugLine(GetWorld(), Start, End, FColor::Yellow, false, 2.0f);
        DrawDebugBox(GetWorld(), Hit.ImpactPoint, FVector(10, 10, 10), FColor::Red, false, 2.0f);
        DrawDebugBox(GetWorld(), VaultLandingHit.ImpactPoint, FVector(10, 10, 10), FColor::Green, false, 2.0f);
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

bool UClimbableDetectorComponent::TraceHead(FHitResult& OutHit)
{
    FVector Start = OwnerCharacter->GetActorLocation() + FVector(0, 0, VerticalTraceHeight * 0.5f);
    FVector End = Start + OwnerCharacter->GetActorUpVector() * UpTraceHeight;

    if (bDebugDraw)
    {
        DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, 2.f, 0, 2.f);
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

