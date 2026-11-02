void FCommonActionRouteNode::CollectRoutedWidgets(TArray<UWidget*>& OutWidgets)
{
    if (!IsRoutingActive())
        return;
    for (int32 Index = 0; Index < RoutedWidgets.Num(); ++Index)
    {
        if (UWidget* RoutedWidget = RoutedWidgets[Index].Get())
            OutWidgets.Add(RoutedWidget);
        else
            RoutedWidgets.RemoveAt(Index--);
    }
    for (const TSharedPtr<FCommonActionRouteNode>& Child : Children)
        Child->CollectRoutedWidgets(OutWidgets);
}
