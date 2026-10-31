#include "CranePanelWidget.h"
#include "CraneTelemetryComponent.h"
#include "Rendering/DrawElements.h"
#include "Styling/CoreStyle.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/SLeafWidget.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SSlider.h"
#include "Widgets/Input/SSpinBox.h"
#include "Widgets/Text/STextBlock.h"

namespace CraneUI
{
    const FLinearColor Background = FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("0B1119")));
    const FLinearColor Surface = FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("131E2A")));
    const FLinearColor Inset = FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("0E1721")));
    const FLinearColor Line = FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("263849")));
    const FLinearColor Muted = FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("8297AA")));
    const FLinearColor White = FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("E3EDF5")));
    const FLinearColor Teal = FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("48DFC0")));
    const FLinearColor Amber = FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("FFBF59")));
    const FLinearColor Red = FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("FF6574")));
    FSlateFontInfo Font(int32 Size, bool Bold = false) { return FCoreStyle::GetDefaultFontStyle(Bold ? "Bold" : "Regular", Size); }
    FText Text(const FString& S) { return FText::FromString(S); }
    FLinearColor Severity(int32 S) { return S == 2 ? Red : S == 1 ? Amber : Teal; }
    TSharedRef<STextBlock> Label(const FString& S, int32 Size = 11, FLinearColor Color = Muted, bool Bold = false)
    {
        return SNew(STextBlock).Text(Text(S)).Font(Font(Size, Bold)).ColorAndOpacity(Color);
    }
    TSharedRef<SBorder> Card(TSharedRef<SWidget> Content, float Padding = 18.f)
    {
        return SNew(SBorder).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush")).BorderBackgroundColor(Surface).Padding(Padding)[Content];
    }
}

/** Vector schematic stays sharp at every viewport size; all geometry follows the telemetry. */
class SCraneDiagram : public SLeafWidget
{
public:
    SLATE_BEGIN_ARGS(SCraneDiagram) {} SLATE_ARGUMENT(TWeakObjectPtr<UCraneTelemetryComponent>, Model) SLATE_END_ARGS()
    void Construct(const FArguments& Args) { Model = Args._Model; }
    virtual FVector2D ComputeDesiredSize(float) const override { return FVector2D(940, 368); }
    virtual bool ComputeVolatility() const override { return true; }
    virtual int32 OnPaint(const FPaintArgs&, const FGeometry& G, const FSlateRect&, FSlateWindowElementList& Out, int32 Layer, const FWidgetStyle&, bool) const override
    {
        using namespace CraneUI;
        if (!Model.IsValid()) return Layer;
        const auto& R = Model->Readings;
        const float SX = G.GetLocalSize().X / 940.f, SY = G.GetLocalSize().Y / 368.f;
        auto Point = [&](float X, float Y) { return FVector2D(X * SX, Y * SY); };
        auto Stroke = [&](TArray<FVector2D> P, FLinearColor C, float Width = 1.f)
        {
            for (FVector2D& V : P) V = Point(V.X, V.Y);
            FSlateDrawElement::MakeLines(Out, Layer + 1, G.ToPaintGeometry(), P, ESlateDrawEffect::None, C, true, Width);
        };
        auto Box = [&](float X, float Y, float W, float H, FLinearColor C)
        {
            FSlateDrawElement::MakeBox(Out, Layer, G.ToPaintGeometry(Point(W,H), FSlateLayoutTransform(Point(X,Y))), FCoreStyle::Get().GetBrush("WhiteBrush"), ESlateDrawEffect::None, C);
        };
        auto Caption = [&](float X, float Y, FString S, FLinearColor C = Muted, int32 Size = 10)
        {
            FSlateDrawElement::MakeText(Out, Layer + 2, G.ToPaintGeometry(Point(200,24), FSlateLayoutTransform(Point(X,Y))), S, Font(Size), ESlateDrawEffect::None, C);
        };
        Box(0,0,940,368,Inset);
        for (int X=20; X<940; X+=32) Stroke({{double(X),0},{double(X),368}}, Line.CopyWithNewOpacity(.28f));
        for (int Y=16; Y<368; Y+=32) Stroke({{0,double(Y)},{940,double(Y)}}, Line.CopyWithNewOpacity(.28f));
        Caption(20,14,TEXT("01 / БОКОВАЯ ПРОЕКЦИЯ"));
        Caption(20,37,TEXT("Кинематическая схема • размеры условные"),Muted,9);
        const float Offset = (R.TravelMetres - 64.f) * .22f;
        const float Base = 215.f + Offset;
        const float TipX = Base + R.RadiusMetres * 10.f;
        const float TipY = 64.f;
        const float HookY = 292.f - R.HookMetres * 6.f;
        const FLinearColor Signal = Severity(Model->GetSeverity());
        Stroke({{28,298},{680,298}},Muted,2);
        Stroke({{28,307},{680,307}},Line,2);
        for (int X=40; X<680; X+=24) Stroke({{double(X),298},{double(X-6),312}},Line,2);
        // Portal, slewing platform, tower, counterweight and lattice boom.
        Stroke({{Base-65,292},{Base-48,206},{Base+44,206},{Base+64,292}},White,6);
        Stroke({{Base-48,210},{Base+44,255},{Base-58,255},{Base+44,210}},Muted,2);
        Box(Base-77,286,29,12,Muted); Box(Base+49,286,29,12,Muted);
        Box(Base-54,192,108,13,Teal);
        Box(Base-30,159,61,30,Line);
        Box(Base-20,163,24,16,Teal.CopyWithNewOpacity(.45f));
        Stroke({{Base-12,159},{Base-8,91},{Base+18,91},{Base+28,159}},White,3);
        Stroke({{Base-7,91},{Base+26,156},{Base-10,156},{Base+17,93}},Muted,1.5f);
        Box(Base-99,153,50,32,Muted);
        Stroke({{Base-95,151},{Base+8,94},{TipX,TipY}},Muted,1.5f);
        const FVector2D A(Base+21,174), B(TipX,TipY);
        const FVector2D Normal = FVector2D(-(B-A).Y,(B-A).X).GetSafeNormal()*13;
        Stroke({A-Normal,B-Normal,B+Normal,A+Normal,A-Normal},Teal,3);
        for(int I=0; I<9; ++I)
        {
            const FVector2D P = FMath::Lerp(A,B,I/9.0);
            const FVector2D Q = FMath::Lerp(A,B,(I+1)/9.0);
            Stroke({P-Normal,Q+Normal,Q-Normal},Teal.CopyWithNewOpacity(.65f),1.5f);
        }
        Stroke({{TipX-3,TipY},{TipX-3,HookY},{TipX+3,HookY},{TipX+3,TipY}},White,1.5f);
        Stroke({{TipX,HookY},{TipX,HookY+10},{TipX+7,HookY+14},{TipX+12,HookY+8}},Signal,3);
        Box(TipX-23,HookY+19,46,29,Signal.CopyWithNewOpacity(.2f));
        Stroke({{TipX-23,HookY+19},{TipX+23,HookY+19},{TipX+23,HookY+48},{TipX-23,HookY+48},{TipX-23,HookY+19}},Signal,1.5f);
        Caption(TipX+33,HookY+20,FString::Printf(TEXT("%.1f т"),R.LoadTonnes),Signal,13);
        Stroke({{Base,326},{TipX,326}},Teal,1);
        Stroke({{Base,320},{Base,332}},Teal,1); Stroke({{TipX,320},{TipX,332}},Teal,1);
        Caption((Base+TipX)/2-45,335,FString::Printf(TEXT("R  %.1f м"),R.RadiusMetres),Teal,11);
        Stroke({{TipX+91,HookY},{TipX+91,292}},Muted,1);
        Caption(TipX+102,232,FString::Printf(TEXT("H %.1f м"),R.HookMetres),White,10);
        // Plan view makes the slew angle observable without rotating the side elevation.
        Box(716,14,208,218,Surface);
        Caption(732,27,TEXT("02 / ПОВОРОТ"));
        const FVector2D Center(820,125);
        TArray<FVector2D> Circle;
        for(int I=0; I<=64; ++I) { const double Angle=I*2*PI/64; Circle.Add(Center+FVector2D(FMath::Sin(Angle),-FMath::Cos(Angle))*58); }
        Stroke(Circle,Line,2);
        Stroke({{820,60},{820,190}},Line); Stroke({{755,125},{885,125}},Line);
        const double Angle = FMath::DegreesToRadians(R.SlewDegrees);
        const FVector2D End = Center+FVector2D(FMath::Sin(Angle),-FMath::Cos(Angle))*53;
        Stroke({Center,End},Teal,4);
        Box(815,120,10,10,White);
        Caption(804,49,TEXT("0°"),Muted,9);
        Caption(783,199,FString::Printf(TEXT("%.1f°"),R.SlewDegrees),Teal,16);
        Caption(737,255,TEXT("ПОЛОЖЕНИЕ НА ПУТЯХ"),Muted,9);
        Caption(737,276,FString::Printf(TEXT("%.1f / 200 м"),R.TravelMetres),White,16);
        Box(737,314,166,3,Line); Box(737,314,166*R.TravelMetres/200.f,3,Teal);
        return Layer + 2;
    }
private:
    TWeakObjectPtr<UCraneTelemetryComponent> Model;
};

/** Draw the fraction directly so the instrument does not depend on progress-bar theme textures. */
class SCraneLoadBar : public SLeafWidget
{
public:
    SLATE_BEGIN_ARGS(SCraneLoadBar) {} SLATE_ARGUMENT(TWeakObjectPtr<UCraneTelemetryComponent>, Model) SLATE_END_ARGS()
    void Construct(const FArguments& Args) { Model = Args._Model; }
    virtual FVector2D ComputeDesiredSize(float) const override { return FVector2D(200, 6); }
    virtual bool ComputeVolatility() const override { return true; }
    virtual int32 OnPaint(const FPaintArgs&, const FGeometry& G, const FSlateRect&, FSlateWindowElementList& Out, int32 Layer, const FWidgetStyle&, bool) const override
    {
        const FSlateBrush* Brush = FCoreStyle::Get().GetBrush("WhiteBrush");
        FSlateDrawElement::MakeBox(Out, Layer, G.ToPaintGeometry(), Brush, ESlateDrawEffect::None, CraneUI::Line);
        if (Model.IsValid())
        {
            const float Percent = Model->GetUtilization();
            const FVector2D Size(G.GetLocalSize().X * FMath::Clamp(Percent / 100.f, 0.f, 1.f), G.GetLocalSize().Y);
            if (Size.X > 0)
            {
                FSlateDrawElement::MakeBox(Out, Layer + 1, G.ToPaintGeometry(Size, FSlateLayoutTransform()), Brush,
                    ESlateDrawEffect::None, CraneUI::Severity(Percent >= 100.f ? 2 : Percent >= 90.f ? 1 : 0));
            }
        }
        return Layer + 1;
    }
private:
    TWeakObjectPtr<UCraneTelemetryComponent> Model;
};

class SCranePanel : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SCranePanel) {} SLATE_ARGUMENT(TWeakObjectPtr<UCraneTelemetryComponent>, Model) SLATE_END_ARGS()
    void Construct(const FArguments& Args)
    {
        using namespace CraneUI;
        Model = Args._Model;
        TSharedRef<SVerticalBox> Layout = SNew(SVerticalBox);
        Layout->AddSlot().AutoHeight().Padding(0,0,0,22)
        [ SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(1)[SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight()[Label(TEXT("PORTAL / CONTROL"),25,White,true)]
                + SVerticalBox::Slot().AutoHeight().Padding(0,5,0,0)[Label(TEXT("ПОРТАЛЬНЫЙ КРАН     /     ПАНЕЛЬ ОПЕРАТОРА     /     ПК–01"),10)]]
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)[SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right)[Label(TEXT("●  ЛОКАЛЬНАЯ СИМУЛЯЦИЯ"),11,Teal,true)]
                + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right).Padding(0,6,0,0)
                [SNew(STextBlock).Font(Font(10)).ColorAndOpacity(Muted).Text_Lambda([]{return Text(FDateTime::Now().ToString(TEXT("%d.%m.%Y   /   %H:%M:%S")));})]]
        ];
        Layout->AddSlot().AutoHeight().Padding(0,0,0,14)[BuildStatus()];
        TSharedRef<SHorizontalBox> Metrics = SNew(SHorizontalBox);
        Metrics->AddSlot().FillWidth(1.15f).Padding(0,0,12,0)[Metric(TEXT("НАГРУЗКА НА КРЮКЕ"),0,TEXT("т"),TEXT("Текущая масса груза"))];
        Metrics->AddSlot().FillWidth(1).Padding(0,0,12,0)[Metric(TEXT("ВЫЛЕТ СТРЕЛЫ"),1,TEXT("м"),TEXT("Диапазон 8–32 м"))];
        Metrics->AddSlot().FillWidth(1).Padding(0,0,12,0)[Metric(TEXT("ВЫСОТА КРЮКА"),2,TEXT("м"),TEXT("Диапазон 2–30 м"))];
        Metrics->AddSlot().FillWidth(1)[Metric(TEXT("СКОРОСТЬ ВЕТРА"),5,TEXT("м/с"),TEXT("Предупреждение от 15 м/с"))];
        Layout->AddSlot().AutoHeight().Padding(0,0,0,14)[Metrics];
        Layout->AddSlot().FillHeight(1).Padding(0,0,0,14)
        [SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(1).Padding(0,0,14,0)[BuildDiagram()]
            + SHorizontalBox::Slot().AutoWidth()[SNew(SBox).WidthOverride(330)[BuildControls()]]
        ];
        Layout->AddSlot().AutoHeight().Padding(0,0,0,12)[BuildJournal()];
        Layout->AddSlot().AutoHeight()[SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(1)[Label(TEXT("УЧЕБНЫЙ СТЕНД   /   Условная грузовая характеристика. Не является прибором безопасности."),9)]
            + SHorizontalBox::Slot().AutoWidth()[Label(TEXT("UNREAL ENGINE 5.7     •     v1.0"),9)]];
        ChildSlot[SNew(SBorder).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush")).BorderBackgroundColor(Background).Padding(0)
            [SNew(SScaleBox).Stretch(EStretch::ScaleToFit)[SNew(SBox).WidthOverride(1440).HeightOverride(960)
                [SNew(SBorder).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush")).BorderBackgroundColor(Background).Padding(28)[Layout]]]]];
    }
private:
    TWeakObjectPtr<UCraneTelemetryComponent> Model;

    TSharedRef<SWidget> BuildStatus()
    {
        using namespace CraneUI;
        return SNew(SBorder).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
            .BorderBackgroundColor_Lambda([this]{return Severity(Model->GetSeverity()).CopyWithNewOpacity(.12f);}).Padding(FMargin(16,10))
            [SNew(SHorizontalBox)
                + SHorizontalBox::Slot().FillWidth(1).VAlign(VAlign_Center)
                [SNew(STextBlock).Font(Font(12,true)).ColorAndOpacity_Lambda([this]{return Severity(Model->GetSeverity());})
                    .Text_Lambda([this]{return Text(TEXT("●  ")+Model->GetStatus()+(Model->bAcknowledged ? TEXT("  •  ПОДТВЕРЖДЕНО") : TEXT("")));})]
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
                [SNew(STextBlock).Font(Font(10)).ColorAndOpacity(Muted).Text_Lambda([this]{return Text(Model->bEmergencyStop ? TEXT("СБРОС ДЛЯ ПРОДОЛЖЕНИЯ") : Model->bCycle ? (Model->bPaused ? TEXT("ЦИКЛ / ПАУЗА") : TEXT("ЦИКЛ / ВОСПРОИЗВЕДЕНИЕ")) : TEXT("РУЧНОЕ УПРАВЛЕНИЕ"));})]];
    }

    TSharedRef<SWidget> Metric(FString Title, int32 Index, FString Unit, FString Hint)
    {
        using namespace CraneUI;
        return Card(SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight()[Label(Title,10)]
            + SVerticalBox::Slot().AutoHeight().Padding(0,5,0,4)
            [SNew(STextBlock).Font(Font(32,true)).ColorAndOpacity_Lambda([this,Index]{ return Index == 0 ? Severity(Model->GetUtilization()>=100 ? 2 : Model->GetUtilization()>=90 ? 1 : 0) : Index==5 ? Severity(Model->Readings.WindMps>=20 ? 2 : Model->Readings.WindMps>=15 ? 1 : 0) : White; })
                .Text_Lambda([this,Index,Unit]{return Text(FString::Printf(TEXT("%.1f  %s"),Model->GetParameter(Index),*Unit));})]
            + SVerticalBox::Slot().AutoHeight()[Label(Hint,9)],16);
    }

    TSharedRef<SWidget> Button(FString Title, TFunction<void()> Action, FLinearColor Color, TAttribute<bool> Enabled = true)
    {
        using namespace CraneUI;
        return SNew(SButton).ButtonColorAndOpacity(Line).ContentPadding(FMargin(12,8)).HAlign(HAlign_Center)
            .IsEnabled(Enabled).OnClicked_Lambda([Action]{Action(); return FReply::Handled();})[Label(Title,10,Color,true)];
    }

    TSharedRef<SWidget> BuildDiagram()
    {
        using namespace CraneUI;
        TSharedRef<SHorizontalBox> Scenarios = SNew(SHorizontalBox);
        const TCHAR* Names[] = {TEXT("Норма"),TEXT("Предел 95%"),TEXT("Перегрузка"),TEXT("Сильный ветер"),TEXT("Автоцикл")};
        for(int32 I=0; I<5; ++I)
            Scenarios->AddSlot().FillWidth(1).Padding(0,0,I==4?0:8,0)[Button(Names[I],[this,I]{Model->ApplyScenario(static_cast<ECraneScenario>(I));},I==4?Teal:White,TAttribute<bool>::CreateLambda([this]{return !Model->bEmergencyStop;}))];
        return Card(SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight().Padding(0,0,0,12)[SNew(SHorizontalBox)
                + SHorizontalBox::Slot().FillWidth(1)[Label(TEXT("ОБЗОР КРАНА"),11,White,true)]
                + SHorizontalBox::Slot().AutoWidth()[Label(TEXT("●  ДАННЫЕ В РЕАЛЬНОМ ВРЕМЕНИ"),9,Teal)]]
            + SVerticalBox::Slot().FillHeight(1)[SNew(SCraneDiagram).Model(Model)]
            + SVerticalBox::Slot().AutoHeight().Padding(0,12,0,8)[Label(TEXT("ДЕМОНСТРАЦИОННЫЕ СЦЕНАРИИ"),9)]
            + SVerticalBox::Slot().AutoHeight()[Scenarios],16);
    }

    TSharedRef<SWidget> BuildControls()
    {
        using namespace CraneUI;
        TSharedRef<SVerticalBox> Controls = SNew(SVerticalBox);
        Controls->AddSlot().AutoHeight().Padding(0,0,0,8)[Label(TEXT("ПАРАМЕТРЫ СИМУЛЯЦИИ"),11,White,true)];
        Controls->AddSlot().AutoHeight().Padding(0,0,0,14)[Label(TEXT("Ползунок или точный ввод значения"),9)];
        Controls->AddSlot().AutoHeight()[SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(1)[Label(TEXT("ЗАГРУЗКА"),9)]
            + SHorizontalBox::Slot().AutoWidth()[SNew(STextBlock).Font(Font(12,true)).ColorAndOpacity_Lambda([this]{return Severity(Model->GetSeverity());})
                .Text_Lambda([this]{return Text(FString::Printf(TEXT("%.0f %%"),Model->GetUtilization()));})]];
        Controls->AddSlot().AutoHeight().Padding(0,6,0,5)[SNew(SBox).HeightOverride(6)[SNew(SCraneLoadBar).Model(Model)]];
        Controls->AddSlot().AutoHeight().Padding(0,0,0,12)[SNew(STextBlock).Font(Font(9)).ColorAndOpacity(Muted)
            .Text_Lambda([this]{return Text(FString::Printf(TEXT("Допустимо %.1f т  •  сигнал от 90%%"),Model->GetCapacity()));})];
        const TCHAR* Names[] = {TEXT("Груз, т"),TEXT("Вылет, м"),TEXT("Крюк, м"),TEXT("Поворот, °"),TEXT("Путь, м"),TEXT("Ветер, м/с")};
        const float Min[] = {0,8,2,0,0,0}, Max[] = {40,32,30,360,200,30};
        for(int32 I=0;I<6;++I)
        {
            Controls->AddSlot().AutoHeight().Padding(0,0,0,5)[SNew(SHorizontalBox)
                + SHorizontalBox::Slot().FillWidth(1).VAlign(VAlign_Center)[Label(Names[I],10,White)]
                + SHorizontalBox::Slot().AutoWidth()[SNew(SBox).WidthOverride(82)[SNew(SSpinBox<float>).Font(Font(10)).MinValue(Min[I]).MaxValue(Max[I]).Delta(.1f).MaxFractionalDigits(1)
                    .Value_Lambda([this,I]{return Model->GetParameter(I);}).OnValueChanged_Lambda([this,I](float V){Model->SetParameter(I,V);})
                    .IsEnabled_Lambda([this]{return !Model->bEmergencyStop;})]]];
            Controls->AddSlot().AutoHeight().Padding(0,0,0,5)[SNew(SBox).HeightOverride(12)[SNew(SSlider).SliderBarColor(Line).SliderHandleColor(Teal)
                .Value_Lambda([this,I,Low=Min[I],High=Max[I]]{return (Model->GetParameter(I)-Low)/(High-Low);})
                .OnValueChanged_Lambda([this,I,Low=Min[I],High=Max[I]](float V){Model->SetParameter(I,FMath::Lerp(Low,High,V));})
                .IsEnabled_Lambda([this]{return !Model->bEmergencyStop;})]];
        }
        Controls->AddSlot().FillHeight(1)[SNew(SBox)];
        Controls->AddSlot().AutoHeight().Padding(0,0,0,8)[SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(1).Padding(0,0,8,0)[SNew(SButton).ButtonColorAndOpacity(Line).HAlign(HAlign_Center).ContentPadding(FMargin(8))
                .IsEnabled_Lambda([this]{return Model->bCycle && !Model->bEmergencyStop;}).OnClicked_Lambda([this]{Model->TogglePause();return FReply::Handled();})
                [SNew(STextBlock).Font(Font(10,true)).ColorAndOpacity(White).Text_Lambda([this]{return Text(Model->bPaused?TEXT("Продолжить"):TEXT("Пауза"));})]]
            + SHorizontalBox::Slot().FillWidth(1)[Button(TEXT("Сброс"),[this]{Model->Reset();},White)]];
        Controls->AddSlot().AutoHeight()[Button(TEXT("■   АВАРИЙНЫЙ СТОП"),[this]{Model->EmergencyStop();},Red,TAttribute<bool>::CreateLambda([this]{return !Model->bEmergencyStop;}))];
        return Card(Controls,18);
    }

    TSharedRef<SWidget> BuildJournal()
    {
        using namespace CraneUI;
        return Card(SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(1)[SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight().Padding(0,0,0,8)[Label(TEXT("ЖУРНАЛ СОБЫТИЙ   /   ПОСЛЕДНИЕ 3 ИЗ 64"),10,White,true)]
                + SVerticalBox::Slot().AutoHeight()[SNew(STextBlock).Font(Font(10)).ColorAndOpacity(Muted).Text_Lambda([this]
                    {
                        TArray<FString> Last;
                        for(int32 I=0; I<FMath::Min(3,Model->Events.Num()); ++I) Last.Add(Model->Events[I]);
                        return Text(FString::Join(Last,TEXT("\n")));
                    })]]
            + SHorizontalBox::Slot().AutoWidth()[SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight()[SNew(SHorizontalBox)
                    + SHorizontalBox::Slot().AutoWidth().Padding(0,0,8,0)[Button(TEXT("Подтвердить"),[this]{Model->Acknowledge();},Amber,TAttribute<bool>::CreateLambda([this]{return Model->GetSeverity()>0 && !Model->bAcknowledged;}))]
                    + SHorizontalBox::Slot().AutoWidth()[Button(TEXT("Экспорт CSV"),[this]{Model->ExportLog();},White)]]
                + SVerticalBox::Slot().AutoHeight().Padding(0,8,0,0)[SNew(STextBlock).Font(Font(9)).ColorAndOpacity(Muted).Text_Lambda([this]{return Text(Model->ExportMessage);})]],16);
    }
};

UCranePanelWidget::UCranePanelWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    SetIsFocusable(true);
}

TSharedRef<SWidget> UCranePanelWidget::RebuildWidget()
{
    if (!IsValid(Telemetry)) return SNew(STextBlock).Text(FText::FromString(TEXT("Назначьте компонент Telemetry перед AddToViewport.")));
    return SNew(SCranePanel).Model(Telemetry);
}
