// Include CommonActionBindingRoute.h in the existing registry translation unit.
// This body uses that translation unit's existing Records map, not another map.
void CommonActionBindingData::CompactInvalidBindings()
{
    for (auto It = Records.CreateIterator(); It; ++It)
    {
        auto& Record = It.Value();
        if (Record.Owner.IsValid() && Record.Click.IsBound()) continue;
        if (Record.Route.IsValid())
        {
            const auto Route = Record.Route.Pin();
            Route->Disassociate(It.Key());
        }
        It.RemoveCurrent();
    }
}
