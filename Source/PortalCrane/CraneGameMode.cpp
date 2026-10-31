#include "CraneGameMode.h"
#include "CraneTelemetryComponent.h"
#include "CranePanelWidget.h"
#include "Engine/World.h"
#include "HighResScreenshot.h"
#include "UnrealClient.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "TimerManager.h"

ACraneGameMode::ACraneGameMode()
{
    PlayerControllerClass = ACranePlayerController::StaticClass();
    DefaultPawnClass = nullptr;
}

ACranePlayerController::ACranePlayerController()
{
    Telemetry = CreateDefaultSubobject<UCraneTelemetryComponent>(TEXT("Telemetry"));
}

void ACranePlayerController::BeginPlay()
{
    Super::BeginPlay();
    if (!IsLocalController()) return;
    Panel = CreateWidget<UCranePanelWidget>(this, UCranePanelWidget::StaticClass());
    Panel->Telemetry = Telemetry;
    Panel->AddToViewport();
    bShowMouseCursor = true;
    FInputModeUIOnly InputMode;
    InputMode.SetWidgetToFocus(Panel->TakeWidget());
    InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
    SetInputMode(InputMode);
    FString Scenario;
    if (FParse::Value(FCommandLine::Get(), TEXT("CraneScenario="), Scenario))
    {
        if (Scenario == TEXT("overload")) Telemetry->ApplyScenario(ECraneScenario::Overload);
        if (Scenario == TEXT("wind")) Telemetry->ApplyScenario(ECraneScenario::HighWind);
        if (Scenario == TEXT("cycle")) Telemetry->ApplyScenario(ECraneScenario::Cycle);
    }
    if (FParse::Param(FCommandLine::Get(), TEXT("CraneScreenshot")))
    {
        FTimerHandle Timer;
        GetWorldTimerManager().SetTimer(Timer, []
        {
            FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("Screenshots/CranePanel.png"), true, false);
        }, 5.f, false);
    }
}
