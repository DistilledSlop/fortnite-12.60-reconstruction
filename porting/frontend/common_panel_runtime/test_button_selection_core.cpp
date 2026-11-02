#include "CommonButtonSelectionCore.h"
#include <cassert>
#include <functional>
#include <string>
#include <vector>

struct Operations
{
    bool Selected = false, Selectable = true, Toggleable = false;
    bool Hovered = false, InteractableSelected = false, InteractionFlag = true;
    bool InternalAvailable = true, ButtonEnabled = true;
    int Button = 1;
    std::vector<std::string> Trace;
    std::function<void()> StyleHook, SelectedHook, ResolveHook, BlueprintHook;
    bool IsHovered() { Trace.push_back("hover"); return Hovered; }
    bool IsSelectable() { Trace.push_back("selectable"); return Selectable; }
    bool IsSelected() { Trace.push_back("selected?"); return Selected; }
    bool IsToggleable() { Trace.push_back("toggleable"); return Toggleable; }
    bool IsInteractableWhenSelected() { Trace.push_back("selected-interaction"); return InteractableSelected; }
    bool IsInteractionEnabledFlag() { Trace.push_back("interaction-flag"); return InteractionFlag; }
    bool IsButtonEnabledFlag() { Trace.push_back("button-enabled"); return ButtonEnabled; }
    void SetSelected(bool Value) { Trace.push_back(Value ? "set1" : "set0"); Selected = Value; }
    void RefreshStyle() { Trace.push_back("style"); if (StyleHook) StyleHook(); }
    void OnSelected(bool Notify) { Trace.push_back(Notify ? "on1-notify" : "on1"); if (SelectedHook) SelectedHook(); }
    void OnDeselected(bool Notify) { Trace.push_back(Notify ? "on0-notify" : "on0"); }
    int* ResolveInternalButton() { Trace.push_back("resolve"); if (ResolveHook) ResolveHook(); return InternalAvailable ? &Button : nullptr; }
    void SetInternalStyle(int* Target, CommonButtonSelectionCore::ButtonStyle Style) {
        assert(Target == &Button);
        using CommonButtonSelectionCore::ButtonStyle;
        Trace.push_back(Style == ButtonStyle::Selected ? "style-selected"
                       : Style == ButtonStyle::Normal ? "style-normal" : "style-disabled");
        if (StyleHook) StyleHook();
    }
    void SetInternalInteraction(int* Target, bool Enabled) {
        assert(Target == &Button); Trace.push_back(Enabled ? "interaction1" : "interaction0");
    }
    void PlaySelectionFeedback() { Trace.push_back("feedback"); }
    void RefreshInputAction() { Trace.push_back("input"); }
    void OnHovered() { Trace.push_back("hovered-event"); }
    void OnUnhovered() { Trace.push_back("unhovered-event"); }
    void BlueprintSelected() { Trace.push_back("bp1"); if (BlueprintHook) BlueprintHook(); }
    void BlueprintDeselected() { Trace.push_back("bp0"); if (BlueprintHook) BlueprintHook(); }
    void SinglecastSelected(bool Value) { Trace.push_back(Value ? "single1" : "single0"); }
    void MulticastSelected(bool Value) { Trace.push_back(Value ? "multi1" : "multi0"); }
    void TextStyleChanged() { Trace.push_back("text-style"); }
};

static void Expect(const Operations& Ops, std::initializer_list<const char*> Expected)
{
    std::vector<std::string> Values(Expected.begin(), Expected.end());
    assert(Ops.Trace == Values);
}

int main()
{
    using namespace CommonButtonSelectionCore;
    {
        Operations O; O.InternalAvailable = false;
        RefreshStyle(O);
        Expect(O, {"resolve"});
    }
    for (bool Selected : {false, true}) for (bool Enabled : {false, true})
    {
        Operations O; O.Selected = Selected; O.ButtonEnabled = Enabled;
        RefreshStyle(O);
        std::vector<std::string> Expected{"resolve", "selected?"};
        if (!Selected) Expected.push_back("button-enabled");
        Expected.push_back(Selected ? "style-selected" : Enabled ? "style-normal" : "style-disabled");
        Expected.push_back("text-style");
        assert(O.Trace == Expected);
    }
    {
        Operations O;
        O.ResolveHook = [&] { O.Selected = true; };
        O.StyleHook = [&] { O.Selected = false; };
        RefreshStyle(O);
        Expect(O, {"resolve", "selected?", "style-selected", "text-style"});
        assert(!O.Selected);
    }
    {
        Operations O;
        Set(true, true, O);
        Expect(O, {"hover", "selectable", "selected?", "set1", "style", "selected?",
                   "on1-notify", "toggleable", "resolve", "selected-interaction",
                   "interaction0", "feedback", "input", "hover"});
    }
    {
        Operations O; O.Selectable = false;
        Set(true, true, O);
        Expect(O, {"hover", "selectable", "hover"});
    }
    {
        Operations O; O.Selected = true;
        Set(true, true, O);
        Expect(O, {"hover", "selectable", "selected?", "hover"});
        O.Trace.clear(); Set(false, true, O);
        Expect(O, {"hover", "selectable", "selected?", "toggleable", "hover"});
    }
    {
        Operations O; O.Selected = true; O.Toggleable = true;
        // The public deselection path forces feedback=true even when false was supplied.
        // Reentrant style change back to selected makes this observable.
        O.StyleHook = [&] { O.Selected = true; };
        Set(false, false, O);
        Expect(O, {"hover", "selectable", "selected?", "toggleable", "set0", "style",
                   "selected?", "on1-notify", "toggleable", "feedback", "input", "hover"});
    }
    {
        Operations O; O.Hovered = true;
        O.StyleHook = [&] { O.Selected = false; O.Hovered = false; };
        // The post-style state, rather than requested state, determines the branch.
        Set(true, true, O);
        Expect(O, {"hover", "selectable", "selected?", "set1", "style", "selected?",
                   "resolve", "interaction-flag", "interaction1", "on0-notify", "input",
                   "hover", "unhovered-event"});
    }
    {
        Operations O;
        // OnSelected may change toggleability before its subsequent test.
        O.SelectedHook = [&] { O.Toggleable = true; O.Hovered = true; };
        Set(true, false, O);
        Expect(O, {"hover", "selectable", "selected?", "set1", "style", "selected?",
                   "on1-notify", "toggleable", "input", "hover", "hovered-event"});
    }
    {
        Operations O;
        // The branch is not reselected after OnSelected; later operations retain
        // the selected path even if the event changes the state again.
        O.SelectedHook = [&] { O.Selected = false; };
        SetInternal(true, true, true, O);
        Expect(O, {"set1", "style", "selected?", "on1-notify", "toggleable", "resolve",
                   "selected-interaction", "interaction0", "feedback", "input"});
        assert(!O.Selected);
    }
    {
        Operations O; bool Nested = false;
        O.StyleHook = [&] {
            if (!Nested) { Nested = true; SetInternal(false, false, false, O); }
        };
        SetInternal(true, true, true, O);
        Expect(O, {"set1", "style", "set0", "style", "selected?", "resolve",
                   "interaction-flag", "interaction1", "on0", "input", "selected?",
                   "resolve", "interaction-flag", "interaction1", "on0-notify", "input"});
        assert(!O.Selected);
    }
    {
        Operations O;
        // Interaction flags are read after weak resolution, on both paths.
        O.ResolveHook = [&] { O.InteractableSelected = true; O.InteractionFlag = false; };
        SetInternal(true, false, false, O);
        Expect(O, {"set1", "style", "selected?", "on1", "toggleable", "resolve",
                   "selected-interaction", "interaction1", "input"});
        O.Trace.clear(); SetInternal(false, true, false, O);
        Expect(O, {"set0", "style", "selected?", "resolve", "interaction-flag",
                   "interaction0", "on0", "input"});
    }
    {
        Operations O; O.Selectable = false; O.Selected = false;
        // Clear is unconditional: no public selectability/change/hover gates.
        Clear(O);
        Expect(O, {"set0", "style", "selected?", "resolve", "interaction-flag",
                   "interaction1", "on0-notify", "input"});
    }
    for (bool Selected : {false, true}) for (bool Notify : {false, true})
    {
        Operations O;
        O.BlueprintHook = [&] { O.Selected = !Selected; };
        NotifySelection(Selected, Notify, O);
        std::vector<std::string> Expected{Selected ? "bp1" : "bp0"};
        if (Notify) {
            Expected.push_back(Selected ? "single1" : "single0");
            Expected.push_back(Selected ? "multi1" : "multi0");
        }
        Expected.push_back("text-style");
        assert(O.Trace == Expected);
    }
}
