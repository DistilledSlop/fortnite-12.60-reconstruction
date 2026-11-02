#pragma once
// Portable decision core for 0x1407c2170/0x1407c2550. Not registered as a UObject method.
namespace CommonButtonTextStyleCore
{
enum class Candidate { Normal, NormalHovered, Selected, SelectedHovered, Disabled };

// ResolveOwner returns the typed button-style CDO, or null. TryCandidate must
// lazily read and validate the requested class and its CDO, returning that CDO
// or null; its implementation owns the original candidate's validation reads.
// ClassOf reads the returned CDO's actual class. Enabled and selected are sampled
// once at their original branch points; fallback samples hover again even when
// a selected candidate's resolution has synchronously changed state.
template<class ResolveOwner, class IsEnabled, class IsSelected, class IsHovered,
         class TryCandidate, class ClassOf>
auto Get(ResolveOwner&& Owner, IsEnabled&& Enabled, IsSelected&& Selected,
         IsHovered&& Hovered, TryCandidate&& Try, ClassOf&& ActualClass)
    -> decltype(ActualClass(Try(Owner(), Candidate::Normal)))
{
    using Result = decltype(ActualClass(Try(Owner(), Candidate::Normal)));
    auto style = Owner();
    if (!style) return Result{};
    if (!Enabled()) {
        if (auto object = Try(style, Candidate::Disabled)) return ActualClass(object);
    } else if (Selected()) {
        if (Hovered())
            if (auto object = Try(style, Candidate::SelectedHovered)) return ActualClass(object);
        if (auto object = Try(style, Candidate::Selected)) return ActualClass(object);
    }
    if (Hovered())
        if (auto object = Try(style, Candidate::NormalHovered)) return ActualClass(object);
    if (auto object = Try(style, Candidate::Normal)) return ActualClass(object);
    return Result{};
}
}
