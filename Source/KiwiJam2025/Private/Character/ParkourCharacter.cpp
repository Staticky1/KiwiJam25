// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/ParkourCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Engine/LocalPlayer.h"
#include "Blueprint/UserWidget.h"
#include "UI/WorldMapWidget.h"
#include "GameFramework/PlayerController.h"

DEFINE_LOG_CATEGORY(LogParkourCharacter);

// Sets default values
AParkourCharacter::AParkourCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UParkourMovementComponent>(ACharacter::CharacterMovementComponentName))
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-10.f, 0.f, 60.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = false;

	ClimbableDetectorComponent = CreateDefaultSubobject<UClimbableDetectorComponent>("ClimbableDetector");
	ClimbableDetectorComponent->SetOwnerCharacter(this);
	//ClimbableDetectorComponent->RegisterComponent();

	bMapOpen = false;

}

// Called when the game starts or when spawned
void AParkourCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

void AParkourCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add movement 
		AddMovementInput(GetActorForwardVector(), MovementVector.Y);
		AddMovementInput(GetActorRightVector(), MovementVector.X);
	}
}

void AParkourCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AParkourCharacter::BeginJump(const FInputActionValue& Value)
{
	if (ClimbableDetectorComponent)
	{
		FClimbableSurfaceResult Result;
		FClimbableSurfaceResult VaultResult;

		if (ClimbableDetectorComponent->CheckVaultSurface(VaultResult) && VaultResult.SurfaceType == EClimbableSurfaceType::Vaultable)
		{
			Cast<UParkourMovementComponent>(GetCharacterMovement())->BeginVault(VaultResult);
		}
		else if (ClimbableDetectorComponent->DetectClimbableSurface(Result) && Result.SurfaceType == EClimbableSurfaceType::Ledge)
		{
			Cast<UParkourMovementComponent>(GetCharacterMovement())->BeginClimb(Result);
		}
		else
		{
			Super::Jump();
		}
	}

}

void AParkourCharacter::SetCameraRotation()
{
	FirstPersonCameraComponent->SetWorldRotation(GetControlRotation() + AdditionalCameraRotation);
	AdditionalCameraRotation = FRotator();
}

void AParkourCharacter::ToggleMap(const FInputActionValue& Value)
{
	APlayerController* PC = Cast<APlayerController>(GetController()); 
	if (!PC || !WorldMapWidgetClass) return; 

	if (!bMapOpen)
	{
		// Create and display map
		if (!WorldMapWidget) 
		{
			WorldMapWidget = CreateWidget<UWorldMapWidget>(PC, WorldMapWidgetClass); 
			if (!WorldMapWidget) return;

			// Example: Set world bounds before showing
			FBox MapBounds(FVector(-2000, -2000, 0), FVector(2000, 2000, 0));
			WorldMapWidget->SetWorldBounds(MapBounds);  
		}

		WorldMapWidget->AddToViewport();
		bMapOpen = true;
	}
	else
	{
		if (WorldMapWidget)
		{
			WorldMapWidget->RemoveFromParent();
		}
		bMapOpen = false;
	}
}

// Called every frame
void AParkourCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	SetCameraRotation();
}

void AParkourCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged(); 

	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller)) 
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer())) 
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0); 
		}
	}
}

// Called to bind functionality to input
void AParkourCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	//Super::SetupPlayerInputComponent(PlayerInputComponent);
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AParkourCharacter::BeginJump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AParkourCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AParkourCharacter::Look);

		//Map
		EnhancedInputComponent->BindAction(MapAction, ETriggerEvent::Triggered, this, &AParkourCharacter::ToggleMap);
	}
	else
	{
		UE_LOG(LogParkourCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input Component!"), *GetNameSafe(this));
	}
}

void AParkourCharacter::AddCameraRotation(FRotator Rotation)
{
	AdditionalCameraRotation += Rotation;
}

UWorldMapWidget* AParkourCharacter::GetWorldMapWidget()
{
	return WorldMapWidget;
}

