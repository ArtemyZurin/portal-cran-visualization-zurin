#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CranePanelWidget.generated.h"

class UCraneTelemetryComponent;

UCLASS()
class PORTALCRANE_API UCranePanelWidget : public UUserWidget
{
    GENERATED_BODY()
public:
    UCranePanelWidget(const FObjectInitializer& ObjectInitializer);
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Crane", meta=(ExposeOnSpawn=true))
    TObjectPtr<UCraneTelemetryComponent> Telemetry;
protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;
};
