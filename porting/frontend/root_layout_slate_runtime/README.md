# Root layout Slate lifecycle candidate

This unregistered source candidate supplies the native RebuildWidget and
ReleaseSlateResources lifecycle. The `.inc` fragments need the startup-root
candidate's methods and the declarations in `declarations.json`. Do not add
the three presentation fields twice if the startup candidate is also used. Integrate
`RootLayoutSlateSettings.h` and `settings.definitions.inc` once in the module;
the startup root presentation setter must read `GFortUILetoBorderWidth` rather
than a literal 36. The fragments require SBox, SScaleBox, SOverlay,
ULocalPlayer, UFortLetoLayoutBorder and console-manager includes. Build and
runtime validation remain necessary.

Base table 146fc7bf0 and Athena table 146fc80c8 both install
ReleaseSlateResources 14516c7c0 at +260 and RebuildWidget 145169f10 at +2a8.
The release body calls UUserWidget's base implementation 143d83b50 then resets
strong references at +2e8, +2d8, +2b8 and +2c8, respectively.

Rebuild first constructs an SBox retained at +2e8, then calls UUserWidget's
base RebuildWidget 143d801e0, then constructs an SOverlay with those widgets in
that order. It returns that overlay directly except for the conditional Leto
path. The type joins use SBox literal 1459ecee0, SOverlay 1459ecee8 and
SScaleBox 1459f7328; original slot additions are at 14516a145/14516a178.

The branch 14516a359..14516a3d8 checks GetOwningLocalPlayer (virtual +290),
then ULocalPlayer::IsPrimaryPlayer 1442f92d0, requires a non-primary player,
a valid typed LetoLayoutBorderClass (+2b0), and enabled console gate. The
same player calls appear in the dormancy setter; UE LocalPlayer.cpp:1684
corroborates the predicate. The class getter 1452c5ea0 names
FortLetoLayoutBorder. 143d53500 matches UUserWidget::CreateWidgetInstance for
a UWidget owner (UE UserWidget.cpp:1858): `CreateWidget` with `this`, the
border class and NAME_None. The original takes the widget without a null
check, and stores that Slate widget at +2d8.

The branch creates an SScaleBox, storing it at +2c8, with the original overlay
as content, SelfHitTestInvisible visibility and a configurable user scale.
It wraps that scale box in an SBox at +2b8, then returns an SOverlay with the
box first and the border's Slate widget second. The scale box starts with its
default stretch (EStretch::None), and the SBox starts with zero padding;
the separate presentation setter changes stretch and padding. The original
argument builders initialize SScaleBox stretch at +1d0 to zero (1407936e3)
and SBox padding at +1a8/+1b0 to zero (140792041..48); Rebuild leaves both
untouched. It explicitly sets both SScaleBox alignments to Fill (14516a614),
which differs from the local SScaleBox header defaults of Center. The SBox
alignments are explicitly zeroed (14516a791) and already default to Fill.
The retained +2e8 box's other consumers have not been exhaustively examined;
no delegate is bound in the recovered lifecycle bodies.

Native initializer 1405ffbc0 registers bool reference console variable
`FortUI.EnableLetoBorder`, with empty help, ECVF_Default, default true and
backing byte 147e1b7d0. Console-manager vtable +40 selects the bool reference
implementation. Initializers 1405ffd10 and 1405ffd80 register float reference
variables `FortUI.LetoBorderWidth` (147e1b7d4, default 36) and
`FortUI.LetoBorderedContentScale` (147e1b7d8, default .95). These values are
mutable; settings and setter must share their actual storage.

Run `python3 porting/frontend/root_layout_slate_runtime/verify.py` for bounded
original-image slot and instruction checks. Those checks establish provenance,
not a compile or runtime result.
