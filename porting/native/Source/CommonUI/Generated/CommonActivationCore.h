#pragma once
// Shared by generated UObject methods and portable execution-order tests.
// Original bodies: 1407a8e60/1407b7ed0,1407ce7f0/880,1407d2330/2950.
namespace CommonActivationCore
{
template<class IsActive, class Request>
void Activate(IsActive&& Is, Request&& Run) { if (!Is()) Run(); }
template<class IsActive, class Request>
void Deactivate(IsActive&& Is, Request&& Run) { if (Is()) Run(); }
template<class SetState, class Notify>
void SetActive(bool Active, SetState&& Set, Notify&& Run) { Set(Active); Run(); }
template<class IsActive, class Visibility, class Blueprint, class Native, class Dynamic>
void Notify(bool Active, IsActive&& Is, Visibility&& SetVisibility, Blueprint&& BP, Native&& N, Dynamic&& D)
{
    if (Is() != Active) return;
    SetVisibility();
    BP();
    N();
    D();
}
// Completion is deliberately before the state test/write. Derived completion
// hooks and Blueprint callbacks may synchronously reenter these operations.
template<class Complete, class IsActive, class SetState, class Notify>
void EndIntro(Complete&& Hook, IsActive&& Is, SetState&& Set, Notify&& Run)
{
    Hook();
    if (!Is()) { Set(true); Run(); }
}
template<class ClearIntroed, class Complete, class SetState, class Notify>
void EndOutro(ClearIntroed&& Clear, Complete&& Hook, SetState&& Set, Notify&& Run)
{
    Clear(); Hook(); Set(false); Run();
}
}
