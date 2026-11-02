void FCommonActionRouteNode::CollectRoutedWidgets(TArray<UWidget*>& OutWidgets)
{
    if (!IsRoutingActive())
        return;
    for (int32 Index = 0; Index < RoutedWidgets.Num(); ++Index)
    {
        if (UWidget* Widget = RoutedWidgets[Index].Get())
            OutWidgets.Add(Widget);
        else
            RoutedWidgets.RemoveAt(Index--);
    }
    for (const TSharedPtr<FCommonActionRouteNode>& Child : Children)
        Child->CollectRoutedWidgets(OutWidgets);
}
