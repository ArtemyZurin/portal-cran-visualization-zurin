#include "CraneTelemetryComponent.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"

UCraneTelemetryComponent::UCraneTelemetryComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UCraneTelemetryComponent::BeginPlay()
{
    Super::BeginPlay();
    Record(TEXT("Панель готова. Источник: учебная модель"));
    Refresh();
}

float UCraneTelemetryComponent::GetCapacity() const
{
    // Illustrative constant-moment chart; replace with the actual crane load chart for integration.
    return FMath::Min(32.f, 480.f / FMath::Max(8.f, Readings.RadiusMetres));
}

float UCraneTelemetryComponent::GetUtilization() const { return 100.f * Readings.LoadTonnes / GetCapacity(); }

uint32 UCraneTelemetryComponent::AlarmMask() const
{
    uint32 Mask = 0;
    if (GetUtilization() >= 100.f) Mask |= 1;
    else if (GetUtilization() >= 90.f) Mask |= 2;
    if (Readings.WindMps >= 20.f) Mask |= 4;
    else if (Readings.WindMps >= 15.f) Mask |= 8;
    if (Readings.HookMetres >= 29.f || Readings.HookMetres <= 3.f) Mask |= 16;
    if (bEmergencyStop) Mask |= 32;
    return Mask;
}

int32 UCraneTelemetryComponent::GetSeverity() const
{
    const uint32 Mask = AlarmMask();
    return (Mask & (1 | 4 | 32)) ? 2 : (Mask ? 1 : 0);
}

FString UCraneTelemetryComponent::GetStatus() const
{
    const uint32 Mask = AlarmMask();
    TArray<FString> Messages;
    if (Mask & 32) Messages.Add(TEXT("АВАРИЙНЫЙ СТОП"));
    if (Mask & 1) Messages.Add(TEXT("ПЕРЕГРУЗКА"));
    if (Mask & 2) Messages.Add(TEXT("ПРЕДЕЛ НАГРУЗКИ"));
    if (Mask & 4) Messages.Add(TEXT("ОПАСНЫЙ ВЕТЕР"));
    if (Mask & 8) Messages.Add(TEXT("УСИЛЕНИЕ ВЕТРА"));
    if (Mask & 16) Messages.Add(TEXT("ПРЕДЕЛ ВЫСОТЫ"));
    return Messages.IsEmpty() ? TEXT("СИСТЕМА В НОРМЕ") : FString::Join(Messages, TEXT("  /  "));
}

void UCraneTelemetryComponent::Record(const FString& Message)
{
    Events.Insert(FDateTime::Now().ToString(TEXT("%H:%M:%S")) + TEXT("   ") + Message, 0);
    if (Events.Num() > 64) Events.SetNum(64);
}

void UCraneTelemetryComponent::Refresh()
{
    const uint32 Mask = AlarmMask();
    if (Mask != PreviousAlarms)
    {
        bAcknowledged = false;
        Record(GetStatus());
        PreviousAlarms = Mask;
    }
    OnReadingsChanged.Broadcast();
}

void UCraneTelemetryComponent::SetReadings(FCraneReadings Value)
{
    if (bEmergencyStop) return;
    const float Values[] = { Value.LoadTonnes, Value.RadiusMetres, Value.HookMetres, Value.SlewDegrees, Value.TravelMetres, Value.WindMps };
    for (float V : Values) if (!FMath::IsFinite(V)) return;
    Value.LoadTonnes = FMath::Clamp(Value.LoadTonnes, 0.f, 40.f);
    Value.RadiusMetres = FMath::Clamp(Value.RadiusMetres, 8.f, 32.f);
    Value.HookMetres = FMath::Clamp(Value.HookMetres, 2.f, 30.f);
    Value.SlewDegrees = FMath::Clamp(Value.SlewDegrees, 0.f, 360.f);
    Value.TravelMetres = FMath::Clamp(Value.TravelMetres, 0.f, 200.f);
    Value.WindMps = FMath::Clamp(Value.WindMps, 0.f, 30.f);
    bCycle = false;
    bPaused = false;
    Readings = Value;
    Refresh();
}

float UCraneTelemetryComponent::GetParameter(int32 Index) const
{
    switch (Index)
    {
        case 0: return Readings.LoadTonnes;
        case 1: return Readings.RadiusMetres;
        case 2: return Readings.HookMetres;
        case 3: return Readings.SlewDegrees;
        case 4: return Readings.TravelMetres;
        case 5: return Readings.WindMps;
        default: return 0;
    }
}

void UCraneTelemetryComponent::SetParameter(int32 Index, float Value)
{
    FCraneReadings Next = Readings;
    switch (Index)
    {
        case 0: Next.LoadTonnes = Value; break;
        case 1: Next.RadiusMetres = Value; break;
        case 2: Next.HookMetres = Value; break;
        case 3: Next.SlewDegrees = Value; break;
        case 4: Next.TravelMetres = Value; break;
        case 5: Next.WindMps = Value; break;
        default: return;
    }
    SetReadings(Next);
}

void UCraneTelemetryComponent::ApplyScenario(ECraneScenario Scenario)
{
    if (bEmergencyStop) return;
    Readings = FCraneReadings();
    bPaused = false;
    bCycle = Scenario == ECraneScenario::Cycle;
    CycleTime = 0;
    const TCHAR* Name = TEXT("Норма");
    switch (Scenario)
    {
        case ECraneScenario::NearLimit: Readings.LoadTonnes = GetCapacity() * .95f; Name = TEXT("Предупреждение"); break;
        case ECraneScenario::Overload: Readings.LoadTonnes = 30.f; Name = TEXT("Перегрузка"); break;
        case ECraneScenario::HighWind: Readings.WindMps = 23.f; Name = TEXT("Сильный ветер"); break;
        case ECraneScenario::Cycle: Name = TEXT("Автоматический цикл"); break;
        default: break;
    }
    Record(FString(TEXT("Сценарий: ")) + Name);
    Refresh();
}

void UCraneTelemetryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    if (!bCycle || bPaused || bEmergencyStop) return;
    CycleTime += DeltaTime;
    const float T = static_cast<float>(CycleTime);
    Readings.LoadTonnes = 12.4f + 2.f * FMath::Sin(T * .22f);
    Readings.RadiusMetres = 18.f + 7.f * FMath::Sin(T * .18f);
    Readings.HookMetres = 16.f + 9.f * FMath::Sin(T * .3f);
    Readings.SlewDegrees = FMath::Fmod(45.f + T * 5.f, 360.f);
    Readings.TravelMetres = 64.f + 20.f * FMath::Sin(T * .1f);
    Readings.WindMps = 6.2f + 2.f * FMath::Sin(T * .4f);
    Refresh();
}

void UCraneTelemetryComponent::TogglePause()
{
    if (!bCycle || bEmergencyStop) return;
    bPaused = !bPaused;
    Record(bPaused ? TEXT("Цикл приостановлен") : TEXT("Цикл продолжен"));
}

void UCraneTelemetryComponent::EmergencyStop()
{
    if (bEmergencyStop) return;
    bEmergencyStop = true;
    bCycle = false;
    bPaused = false;
    Refresh();
}

void UCraneTelemetryComponent::Reset()
{
    bEmergencyStop = false;
    bAcknowledged = false;
    ApplyScenario(ECraneScenario::Normal);
    Record(TEXT("Сброс: начальные параметры восстановлены"));
}

void UCraneTelemetryComponent::Acknowledge()
{
    if (GetSeverity() == 0 || bAcknowledged) return;
    bAcknowledged = true;
    Record(TEXT("Предупреждения подтверждены оператором"));
}

bool UCraneTelemetryComponent::ExportLog()
{
    const FString Directory = FPaths::ProjectSavedDir() / TEXT("CraneLogs");
    IFileManager::Get().MakeDirectory(*Directory, true);
    const FString Path = Directory / (TEXT("events-") + FDateTime::Now().ToString(TEXT("%Y%m%d-%H%M%S")) + TEXT(".csv"));
    FString Csv = TEXT("time;event\r\n");
    for (int32 I = Events.Num() - 1; I >= 0; --I)
    {
        FString Message = Events[I].Mid(11).Replace(TEXT("\""), TEXT("\"\""));
        Csv += Events[I].Left(8) + TEXT(";\"") + Message + TEXT("\"\r\n");
    }
    const bool Success = FFileHelper::SaveStringToFile(Csv, *Path, FFileHelper::EEncodingOptions::ForceUTF8);
    ExportMessage = Success ? TEXT("Сохранено: Saved/CraneLogs") : TEXT("Ошибка записи журнала");
    return Success;
}
