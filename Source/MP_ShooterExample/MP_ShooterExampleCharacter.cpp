// Copyright Epic Games, Inc. All Rights Reserved.

#include "MP_ShooterExampleCharacter.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/LocalPlayer.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "MP_ShooterExampleProjectile.h"

#include "Kismet/GameplayStatics.h"
#include "OnlineSessionSettings.h"

#include "OnlineSubsystem.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// AMP_ShooterExampleCharacter

AMP_ShooterExampleCharacter::AMP_ShooterExampleCharacter()
    : CreateSessionCompleteDelegate(
          FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionComplete))
{
    // Set size for collision capsule
    GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

    // Create a CameraComponent
    FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
    FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
    FirstPersonCameraComponent->SetRelativeLocation(FVector(-10.f, 0.f, 60.f)); // Position the camera
    FirstPersonCameraComponent->bUsePawnControlRotation = true;

    // Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
    Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
    Mesh1P->SetupAttachment(FirstPersonCameraComponent);
    Mesh1P->bCastDynamicShadow = false;
    Mesh1P->CastShadow = false;
    // Mesh1P->SetRelativeRotation(FRotator(0.9f, -19.19f, 5.2f));
    Mesh1P->SetRelativeLocation(FVector(-30.f, 0.f, -150.f));

    // online subsystem
    auto onlineSubsystem = IOnlineSubsystem::Get();

    if (onlineSubsystem)
    {
        OnlineSessionPtr = onlineSubsystem->GetSessionInterface();
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(
                -1, 10.f, FColor::Red,
                FString::Printf(TEXT("Found session: %s"), *onlineSubsystem->GetSubsystemName().ToString()));
            OnlineSessionPtr->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);
        }
    }
}

void AMP_ShooterExampleCharacter::BeginPlay()
{
    // Call the base class
    Super::BeginPlay();
}

//////////////////////////////////////////////////////////////////////////// Input

void AMP_ShooterExampleCharacter::SetupPlayerInputComponent(UInputComponent *PlayerInputComponent)
{
    // Set up action bindings
    if (UEnhancedInputComponent *EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        // Jumping
        EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
        EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

        // Moving
        EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this,
                                           &AMP_ShooterExampleCharacter::Move);

        // Looking
        EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this,
                                           &AMP_ShooterExampleCharacter::Look);
    }
    else
    {
        UE_LOG(LogTemplateCharacter, Error,
               TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input "
                    "system. If you intend to use the legacy system, then you will need to update this C++ file."),
               *GetNameSafe(this));
    }
}

void AMP_ShooterExampleCharacter::CreateGameSession()
{
    if (!OnlineSessionPtr.IsValid()) return;

    const auto existingSessionName = OnlineSessionPtr->GetNamedSession(NAME_GameSession);

    if (existingSessionName)
    {
        OnlineSessionPtr->DestroySession(NAME_GameSession);
    }
    //

    TSharedPtr<FOnlineSessionSettings> SessionSettings = MakeShareable(new FOnlineSessionSettings());
    SessionSettings->bIsLANMatch = false;
    SessionSettings->NumPublicConnections = 4;
    SessionSettings->bAllowJoinInProgress = true;
    SessionSettings->bAllowJoinViaPresence = true;
    SessionSettings->bShouldAdvertise = true;
    SessionSettings->bUsesPresence = true;

    const auto firstLocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
    OnlineSessionPtr->CreateSession(*firstLocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, *SessionSettings);
}

void AMP_ShooterExampleCharacter::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
    if (bWasSuccessful)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green,
                                         FString::Printf(TEXT("Created session: %s"), *SessionName.ToString()));
    }
    else
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("Failed to create session"));
    }
}

void AMP_ShooterExampleCharacter::OpenLobby()
{
    if (GetWorld())
    {
        if (GetWorld()->ServerTravel("/Game/FirstPerson/Maps/FirstPersonMap?listen"))
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Blue, TEXT("Loaded level"));
        }
    }
}

void AMP_ShooterExampleCharacter::CallOpenLevel(const FString &IPAdress)
{
    UGameplayStatics::OpenLevel(this, *IPAdress);
}

void AMP_ShooterExampleCharacter::CallClientTravel(const FString &IPAdress)
{

    if (GetGameInstance())
    {
        const auto localPC = GetGameInstance()->GetFirstLocalPlayerController();

        localPC->ClientTravel(IPAdress, ETravelType::TRAVEL_Absolute);
    }
}

void AMP_ShooterExampleCharacter::Move(const FInputActionValue &Value)
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

void AMP_ShooterExampleCharacter::Look(const FInputActionValue &Value)
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