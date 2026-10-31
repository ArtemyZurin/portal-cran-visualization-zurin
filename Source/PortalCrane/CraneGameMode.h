#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerController.h"
#include "CraneGameMode.generated.h"

class UCraneTelemetryComponent;
class UCranePanelWidget;

UCLASS()
class PORTALCRANE_API ACranePlayerController : public APlayerController
{
    GENERATED_BODY()
public:
    ACranePlayerController();
    virtual void BeginPlay() override;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<UCraneTelemetryComponent> Telemetry;
    UPROPERTY() TObjectPtr<UCranePanelWidget> Panel;
};

UCLASS()
class PORTALCRANE_API ACraneGameMode : public AGameModeBase
{
    GENERATED_BODY()
public:
    ACraneGameMode();
};
