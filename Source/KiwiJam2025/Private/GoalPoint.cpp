// Fill out your copyright notice in the Description page of Project Settings.


#include "GoalPoint.h"
#include "GoalPoint.h"
#include "Components/SphereComponent.h"
#include "Components/BillboardComponent.h"
#include "GameFramework/Character.h"
#include "Character/ParkourCharacter.h"
#include "UI/WorldMapWidget.h"

// Sets default values
AGoalPoint::AGoalPoint()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
    CollisionSphere->InitSphereRadius(100.f);
    RootComponent = CollisionSphere;

    IconBillboard = CreateDefaultSubobject<UBillboardComponent>(TEXT("IconBillboard"));
    IconBillboard->SetupAttachment(RootComponent);

    // Enable overlap events
    CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    CollisionSphere->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
    CollisionSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    CollisionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECollisionResponse::ECR_Overlap);
}

// Called when the game starts or when spawned
void AGoalPoint::BeginPlay()
{
	Super::BeginPlay();

    CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &AGoalPoint::OnOverlapBegin); 
	
 
}

void AGoalPoint::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor && OtherActor->IsA(ACharacter::StaticClass()) && bIsActive)
    {
        OnGoalReached.Broadcast(this);
        bIsActive = false;

        if (GoalMarkerWidget)
        {
            if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
            {
                PlayerChar = Cast<AParkourCharacter>(PC->GetPawn());
                if (PlayerChar) 
                {
                    if (PlayerChar->GetWorldMapWidget())
                    {
                        PlayerChar->GetWorldMapWidget()->RemoveMarker(GoalMarkerWidget);
                    }
                }
            }

            GoalMarkerWidget = nullptr; // Clear reference
        }
    }
}

void AGoalPoint::Tick(float DeltaTime)
{
    // Create marker and register with map
    if (GoalMarkerClass && !bMarkerAdded)
    {
        UE_LOG(LogTemp, Warning, TEXT("Attempting to add marker"));
        if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
        {
            GoalMarkerWidget = CreateWidget<UUserWidget>(PC, GoalMarkerClass);


            if (GoalMarkerWidget)
            {
                // Find the player's map widget and register
                if (APawn* PlayerPawn = PC->GetPawn())
                {
                    PlayerChar = Cast<AParkourCharacter>(PlayerPawn);
                    if (PlayerChar)
                    {
                        if (PlayerChar->GetWorldMapWidget())
                        {
                            PlayerChar->GetWorldMapWidget()->AddMarkerPersistent(GoalMarkerWidget, GetGoalLocation());
                            bMarkerAdded = true;
                        }
                    }
                }
            }
        }
    }
}

FVector AGoalPoint::GetGoalLocation() const
{
    return GetActorLocation();
}

