#include "SCommonAnimatedSwitcher.h"
#include "Styling/WidgetStyle.h"
#include "SlateGlobals.h"
#include "Widgets/SNullWidget.h"

void SCommonAnimatedSwitcher::Construct(const FArguments& InArgs)
{
    SWidgetSwitcher::Construct(SWidgetSwitcher::FArguments().WidgetIndex(InArgs._WidgetIndex));
    bTransitionPhase = false;
    PendingWidget.Reset();
    TransitionType = InArgs._TransitionType;
    const uint8 EaseByte = static_cast<uint8>(InArgs._TransitionCurve);
    const ECurveEaseFunction Ease = EaseByte <= 6 ? static_cast<ECurveEaseFunction>(EaseByte)
                                                  : ECurveEaseFunction::Linear;
    HalfTransitionDuration = InArgs._TransitionDuration * 0.5f;
    TransitionSequence.AddCurve(0.0f, HalfTransitionDuration, Ease);
    OnActiveIndexChanged = InArgs._OnActiveIndexChanged;
    OnTransitioningChanged = InArgs._OnTransitioningChanged;
}

void SCommonAnimatedSwitcher::TransitionToIndex(int32 Index, bool bInstantTransition)
{
    PendingWidget = GetWidget(Index);
    if (!PendingWidget.IsValid())
    {
        UE_LOG(LogSlate, Verbose, TEXT("Called SCommonAnimatedSwitcher::TransitionToIndex('%d') to an invalid Index"), Index);
        return;
    }

    const int32 CurrentIndex = GetActiveWidgetIndex();
    const bool bForward = Index > CurrentIndex;
    const bool bBackward = Index < CurrentIndex;
    if (bInstantTransition || HalfTransitionDuration <= 0.0f)
    {
        if (TransitionSequence.IsPlaying() || bForward || bBackward)
        {
            TransitionSequence.JumpToEnd();
            SWidgetSwitcher::SetActiveWidgetIndex(Index);
            OnActiveIndexChanged.ExecuteIfBound(Index);
        }
        return;
    }
    if (TransitionSequence.IsPlaying())
    {
        if ((TransitionSequence.IsInReverse() && bForward)
            || (!TransitionSequence.IsInReverse() && bBackward)
            || (bTransitionPhase && Index == CurrentIndex))
        {
            bTransitionPhase = !bTransitionPhase;
            TransitionSequence.Reverse();
            if (Index == CurrentIndex)
                OnActiveIndexChanged.ExecuteIfBound(Index);
        }
        return;
    }
    if (!bForward && !bBackward)
        return;

    SetVisibility(EVisibility::HitTestInvisible);
    OnTransitioningChanged.ExecuteIfBound(true);
    if (bForward)
        TransitionSequence.Play(AsShared(), false, 0.0f, true);
    else
        TransitionSequence.PlayReverse(AsShared(), false, 0.0f, true);
    const TSharedPtr<SWidget> ActiveWidget = GetActiveWidget();
    bTransitionPhase = ActiveWidget.IsValid() && ActiveWidget != SNullWidget::NullWidget;
    if (!bTransitionPhase)
    {
        SWidgetSwitcher::SetActiveWidgetIndex(Index);
        OnActiveIndexChanged.ExecuteIfBound(Index);
    }
    if (!bTransitionTimerRegistered)
    {
        bTransitionTimerRegistered = true;
        RegisterActiveTimer(0.0f, FWidgetActiveTimerDelegate::CreateSP(
            this, &SCommonAnimatedSwitcher::HandleTransitionTimer));
    }
}

void SCommonAnimatedSwitcher::UpdateTransitionTransform()
{
    const float Lerp = TransitionSequence.GetLerp();
    const float Amount = bTransitionPhase == TransitionSequence.IsInReverse() ? Lerp - 1.0f : Lerp;
    const uint8 Type = static_cast<uint8>(TransitionType);
    if (Type == 0)
        return;
    if (Type == 3)
        SetRenderTransform(FSlateRenderTransform(1.0f + 0.25f * Amount));
    else if (Type == 1)
        SetRenderTransform(FSlateRenderTransform(FVector2D(-200.0f * Amount, 0.0f)));
    else
        SetRenderTransform(FSlateRenderTransform(FVector2D(0.0f, 200.0f * Amount)));
}

EActiveTimerReturnType SCommonAnimatedSwitcher::HandleTransitionTimer(double CurrentTime, float DeltaTime)
{
    UpdateTransitionTransform();
    if (!TransitionSequence.IsPlaying())
    {
        TSharedPtr<SWidget> Target = PendingWidget.Pin();
        if (Target.IsValid() && Target != GetActiveWidget())
        {
            const bool bOldPhase = bTransitionPhase;
            bTransitionPhase = !bTransitionPhase;
            if (bOldPhase)
            {
                SWidgetSwitcher::SetActiveWidgetIndex(GetWidgetIndex(Target.ToSharedRef()));
                PendingWidget.Reset();
                OnActiveIndexChanged.ExecuteIfBound(GetActiveWidgetIndex());
            }
            const TSharedPtr<SWidget> ActiveWidget = GetActiveWidget();
            if (ActiveWidget.IsValid() && ActiveWidget != SNullWidget::NullWidget)
            {
                if (TransitionSequence.IsInReverse())
                    TransitionSequence.PlayReverse(AsShared(), false, 0.0f, true);
                else
                    TransitionSequence.Play(AsShared(), false, 0.0f, true);
            }
        }
        else
        {
            if (!Target.IsValid())
                UE_LOG(LogSlate, Verbose, TEXT("SCommonAnimatedSwitcher Pending Widget became invalid while a transition was happening"));
            PendingWidget.Reset();
        }
    }
    if (TransitionSequence.IsPlaying())
    {
        Invalidate(EInvalidateWidget::Paint);
        return EActiveTimerReturnType::Continue;
    }
    SetVisibility(EVisibility::SelfHitTestInvisible);
    OnTransitioningChanged.ExecuteIfBound(false);
    bTransitionTimerRegistered = false;
    return EActiveTimerReturnType::Stop;
}

int32 SCommonAnimatedSwitcher::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
    const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId,
    const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
    FWidgetStyle Style(InWidgetStyle);
    if (TransitionSequence.IsPlaying())
    {
        const float Lerp = TransitionSequence.GetLerp();
        Style.BlendOpacity(bTransitionPhase != TransitionSequence.IsInReverse() ? 1.0f - Lerp : Lerp);
    }
    return SWidgetSwitcher::OnPaint(Args, AllottedGeometry, MyCullingRect,
                                    OutDrawElements, LayerId, Style, bParentEnabled);
}
