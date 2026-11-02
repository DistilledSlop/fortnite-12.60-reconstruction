#pragma once

#include "CoreMinimal.h"
#include "Animation/CurveSequence.h"
#include "ECommonSwitcherTransition.h"
#include "ETransitionCurve.h"
#include "Widgets/Layout/SWidgetSwitcher.h"

DECLARE_DELEGATE_OneParam(FCommonSwitcherIndexChanged, int32);
DECLARE_DELEGATE_OneParam(FCommonSwitcherTransitioningChanged, bool);

// Original Slate type constructed by CommonActivatableWidgetContainer's RebuildWidget.
class SCommonAnimatedSwitcher : public SWidgetSwitcher
{
public:
    SLATE_BEGIN_ARGS(SCommonAnimatedSwitcher)
        : _WidgetIndex(0)
        , _TransitionType(ECommonSwitcherTransition::FadeOnly)
        , _TransitionCurve(ETransitionCurve::CubicInOut)
        , _TransitionDuration(0.4f)
    {
        _Visibility = EVisibility::SelfHitTestInvisible;
    }
        SLATE_ATTRIBUTE(int32, WidgetIndex)
        SLATE_ARGUMENT(ECommonSwitcherTransition, TransitionType)
        SLATE_ARGUMENT(ETransitionCurve, TransitionCurve)
        SLATE_ARGUMENT(float, TransitionDuration)
        SLATE_EVENT(FCommonSwitcherIndexChanged, OnActiveIndexChanged)
        SLATE_EVENT(FCommonSwitcherTransitioningChanged, OnTransitioningChanged)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);
    void TransitionToIndex(int32 Index, bool bInstantTransition);

protected:
    virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
        const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements,
        int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

private:
    EActiveTimerReturnType HandleTransitionTimer(double CurrentTime, float DeltaTime);
    void UpdateTransitionTransform();
    FCurveSequence TransitionSequence;
    TWeakPtr<SWidget> PendingWidget;
    bool bTransitionPhase = false;
    bool bTransitionTimerRegistered = false;
    ECommonSwitcherTransition TransitionType = ECommonSwitcherTransition::FadeOnly;
    float HalfTransitionDuration = 0.0f;
    FCommonSwitcherIndexChanged OnActiveIndexChanged;
    FCommonSwitcherTransitioningChanged OnTransitioningChanged;
};
