#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CraneTelemetryComponent.generated.h"

UENUM(BlueprintType)
enum class ECraneScenario : uint8 { Normal, NearLimit, Overload, HighWind, Cycle };

USTRUCT(BlueprintType)
struct FCraneReadings
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float LoadTonnes = 12.4f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float RadiusMetres = 18.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float HookMetres = 16.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float SlewDegrees = 45.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float TravelMetres = 64.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float WindMps = 6.2f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCraneReadingsChanged);

/** Educational telemetry only. No connection to machine actuators. All units are SI except load (t). */
UCLASS(ClassGroup=(Crane), meta=(BlueprintSpawnableComponent))
class PORTALCRANE_API UCraneTelemetryComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UCraneTelemetryComponent();
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(BlueprintReadOnly, Category="Crane") FCraneReadings Readings;
    UPROPERTY(BlueprintReadOnly, Category="Crane") bool bCycle = false;
    UPROPERTY(BlueprintReadOnly, Category="Crane") bool bPaused = false;
    UPROPERTY(BlueprintReadOnly, Category="Crane") bool bEmergencyStop = false;
    UPROPERTY(BlueprintReadOnly, Category="Crane") bool bAcknowledged = false;
    UPROPERTY(BlueprintReadOnly, Category="Crane") TArray<FString> Events;
    UPROPERTY(BlueprintAssignable, Category="Crane") FOnCraneReadingsChanged OnReadingsChanged;

    UFUNCTION(BlueprintCallable, Category="Crane") void SetReadings(FCraneReadings Value);
    UFUNCTION(BlueprintCallable, Category="Crane") void SetParameter(int32 Index, float Value);
    UFUNCTION(BlueprintCallable, Category="Crane") void ApplyScenario(ECraneScenario Scenario);
    UFUNCTION(BlueprintCallable, Category="Crane") void TogglePause();
    UFUNCTION(BlueprintCallable, Category="Crane") void EmergencyStop();
    UFUNCTION(BlueprintCallable, Category="Crane") void Reset();
    UFUNCTION(BlueprintCallable, Category="Crane") void Acknowledge();
    UFUNCTION(BlueprintCallable, Category="Crane") bool ExportLog();
    UFUNCTION(BlueprintPure, Category="Crane") float GetCapacity() const;
    UFUNCTION(BlueprintPure, Category="Crane") float GetUtilization() const;
    UFUNCTION(BlueprintPure, Category="Crane") int32 GetSeverity() const;
    UFUNCTION(BlueprintPure, Category="Crane") FString GetStatus() const;
    float GetParameter(int32 Index) const;
    FString ExportMessage;

private:
    double CycleTime = 0;
    uint32 PreviousAlarms = 0;
    uint32 AlarmMask() const;
    void Refresh();
    void Record(const FString& Message);
};
