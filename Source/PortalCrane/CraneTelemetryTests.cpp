#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "CraneTelemetryComponent.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "Engine/World.h"
#include <limits>

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCraneThresholdTest, "PortalCrane.Telemetry.Thresholds", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FCraneThresholdTest::RunTest(const FString& Parameters)
{
    UCraneTelemetryComponent* M = NewObject<UCraneTelemetryComponent>();
    TestEqual(TEXT("Startup normal"), M->GetSeverity(), 0);
    M->SetParameter(1, 32.f);
    TestEqual(TEXT("Capacity responds to radius"), M->GetCapacity(), 15.f);
    M->SetParameter(0, 13.49f);
    TestEqual(TEXT("Below warning"), M->GetSeverity(), 0);
    M->SetParameter(0, 13.5f);
    TestEqual(TEXT("90 percent warning boundary"), M->GetSeverity(), 1);
    M->SetParameter(0, 15.f);
    TestEqual(TEXT("100 percent alarm boundary"), M->GetSeverity(), 2);
    M->ApplyScenario(ECraneScenario::Normal);
    M->SetParameter(5, 15.f);
    TestEqual(TEXT("Wind warning boundary"), M->GetSeverity(), 1);
    M->SetParameter(5, 20.f);
    TestEqual(TEXT("Wind alarm boundary"), M->GetSeverity(), 2);
    M->SetParameter(0, 40.f);
    TestTrue(TEXT("Concurrent alarms preserved"), M->GetStatus().Contains(TEXT("ВЕТЕР")) && M->GetStatus().Contains(TEXT("ПЕРЕГРУЗКА")));
    M->Acknowledge();
    TestTrue(TEXT("Acknowledged"), M->bAcknowledged);
    TestEqual(TEXT("Acknowledgement does not clear alarm"), M->GetSeverity(), 2);
    M->SetParameter(5, 0.f);
    TestFalse(TEXT("Changed alarm needs acknowledgement"), M->bAcknowledged);
    M->Reset();
    M->SetParameter(2, 29.f);
    TestEqual(TEXT("Upper hook warning"), M->GetSeverity(), 1);
    M->SetParameter(2, 3.f);
    TestEqual(TEXT("Lower hook warning"), M->GetSeverity(), 1);
    M->SetParameter(2, 16.f);
    TestEqual(TEXT("Warning clears after recovery"), M->GetSeverity(), 0);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCraneControlTest, "PortalCrane.Telemetry.Controls", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FCraneControlTest::RunTest(const FString& Parameters)
{
    UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
    UCraneTelemetryComponent* M = NewObject<UCraneTelemetryComponent>();
    M->RegisterComponentWithWorld(World);
    M->ApplyScenario(ECraneScenario::Cycle);
    TestTrue(TEXT("Cycle starts"), M->bCycle);
    const float Before = M->Readings.SlewDegrees;
    M->TickComponent(1.f, LEVELTICK_All, &M->PrimaryComponentTick);
    TestTrue(TEXT("Cycle moves indicators"), M->Readings.SlewDegrees > Before);
    M->TogglePause();
    const float Paused = M->Readings.SlewDegrees;
    M->TickComponent(1.f, LEVELTICK_All, &M->PrimaryComponentTick);
    TestEqual(TEXT("Pause holds readings"), M->Readings.SlewDegrees, Paused);
    M->TogglePause();
    M->SetParameter(0, 8.f);
    TestFalse(TEXT("Manual takeover stops cycle"), M->bCycle);
    M->ApplyScenario(ECraneScenario::Cycle);
    M->EmergencyStop();
    const FCraneReadings Stopped = M->Readings;
    M->SetParameter(0, 1.f);
    M->ApplyScenario(ECraneScenario::HighWind);
    M->TickComponent(1.f, LEVELTICK_All, &M->PrimaryComponentTick);
    TestEqual(TEXT("Stop holds load"), M->Readings.LoadTonnes, Stopped.LoadTonnes);
    TestEqual(TEXT("Stop holds wind"), M->Readings.WindMps, Stopped.WindMps);
    TestEqual(TEXT("Stop is alarm"), M->GetSeverity(), 2);
    M->Acknowledge();
    TestTrue(TEXT("Acknowledging cannot release stop"), M->bEmergencyStop);
    M->Reset();
    TestFalse(TEXT("Reset releases stop"), M->bEmergencyStop);
    TestEqual(TEXT("Reset normal"), M->GetSeverity(), 0);
    M->SetParameter(0, -100.f);
    M->SetParameter(1, 100.f);
    TestEqual(TEXT("Clamp negative mass"), M->Readings.LoadTonnes, 0.f);
    TestEqual(TEXT("Clamp radius"), M->Readings.RadiusMetres, 32.f);
    M->SetParameter(1, std::numeric_limits<float>::infinity());
    TestEqual(TEXT("Reject infinity"), M->Readings.RadiusMetres, 32.f);
    for (int I=0; I<80; ++I) M->ApplyScenario(I%2 ? ECraneScenario::Normal : ECraneScenario::Overload);
    TestEqual(TEXT("Bounded event journal"), M->Events.Num(), 64);
    M->UnregisterComponent();
    World->DestroyWorld(false);
    return true;
}
#endif
