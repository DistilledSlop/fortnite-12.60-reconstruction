Five behavior drafts, including two proposed for counting, plus an uncounted Place rough draft. No canonical files were changed. The manifest has no existing exact class/function entries for these six names. Decompilation inputs are in porting/generated/decomp and decomp_improved; the additional timer callback is retained under decomp/.

Integration declarations required (names proposed, offsets from native code):

- +0x11f8: script multicast MetricInformationChanged with parameters (AFortPlayerStateAthena*, FDetailedMetricInformation); +0x1208: script multicast SimpleMetricInformationChanged with parameters (AFortPlayerStateAthena*, FSimpleMetricInformation). Both broadcasts use by-value payload copies.
- +0x13e8: native multicast PreserveSquadChangedNative(bool); +0x1340: native multicast ResurrectionChipChangedNative(AFortPlayerStateAthena*, bool).
- +0xd70: FTimerHandle TeamMemberStateTimer; +0xda0: native multicast TeamMemberStateChangedNative(ETeamMemberState). Team timer binds a weak target and is one-shot. Callback 0x141a58b00 is reproduced inline in the draft; authority checks the role byte at +0xf0 for value 3.

Place has dynamic broadcast, status broadcast, owner class check and world service update branches, but exact types and semantics for the latter helpers remain unresolved; it is deliberately excluded from manifest entries. Evidence records complete 26-byte exec thunks, body prefix pins, and the complete timer callback, rather than claiming the body prefix is a full function. No builds or tests were run.

Acceptance revision: only PreserveSquad and ResurrectionChip are proposed for counting; their concrete multicast declarations and member proposals are in delegate-declarations.h and member-field-entries.json. Their body-prefix byte pins show register argument and storage offsets; the raw decompilation shows lock count, reverse invocation, and compaction. The two metric drafts are unaccepted because the payload's declared by-value versus reference signature has not been established, notwithstanding the serialized parameter snapshot. The team-state draft is also unaccepted: its byte argument does not prove whether the C++ delegate parameter was uint8 or ETeamMemberState. These three are listed separately in unaccepted-entries.json and must not be integrated as source-complete until signatures are established.

Root integration revision (batch68): the team-state delegate now uses
ETeamMemberState as a source reconstruction choice. The reflected state has that
one-byte enum type; the native broadcaster forwards the byte unchanged, so this
retains values beyond boolean true/false. Original C++ spelling is not claimed.
Replaced the illustrative weak lambda with CreateUObject bound to recovered
NativeExpireTeamMemberState (141a58b00), matching original native object-method
binding. The OnRep, expiry callback, two boolean broadcasts and four native
fields are integrated. Metric payload signature/lifecycle drafts remain unaccepted.
