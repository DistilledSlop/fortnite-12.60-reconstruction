// Incomplete rough lift; exec 0x1429203f0, generated/decomp/AFortGameStateAthena/OnRep_CurrentPlaylistInfo.c.
// Pseudo types and unresolved externs are placeholders, not recovered declarations.
#include <cstdint>
struct Name { uint64_t value; }; struct FString; struct World; struct Listener; struct ListenerMember; struct Analytics;
struct Playlist { Name Name_at_34; uint32_t Field_at_3c, Field_at_58; };
struct GameState {
    Playlist *OverridePlaylist, *BasePlaylist; uint8_t LocalRole;
    uint32_t Field_at_300; int32_t TotalFinalCountdownTime;
    World* GetWorld();
    Playlist* Current() { return OverridePlaylist ? OverridePlaylist : BasePlaylist; }
};
extern void LogNullPlaylist(), LogPlaylistName(Name);
extern int32_t ListenerCount(); extern Listener* ListenerAt(int32_t);
extern bool IsClass_142989f60(Listener*); extern ListenerMember* Member_258(Listener*);
extern FString* StringFromField_141f3b040(uint32_t); extern void FreeString(FString*);
extern void UpdateMember_1417efc70(ListenerMember*, Name, const FString*);
extern Analytics* GetAnalytics_14181af10(); extern void SetAnalytics(Analytics*, Name, const FString*);
extern float EvaluatePlaylist_14070fe00(Playlist*, float, float);
extern void Call_14199f770(GameState*);

void OnRep_CurrentPlaylistInfo_draft(GameState* self)
{
    Playlist* selected = self->Current();
    if (!selected) LogNullPlaylist(); else LogPlaylistName(selected->Name_at_34);
    if (self->GetWorld()) {
        for (int32_t i = 0; i < ListenerCount(); ++i) {
            Listener* listener = ListenerAt(i);
            if (!listener || !IsClass_142989f60(listener)) continue;
            ListenerMember* member = Member_258(listener);
            if (!member) continue;
            FString* mode = StringFromField_141f3b040(selected ? selected->Field_at_3c : 8);
            Playlist* current = self->Current();
            UpdateMember_1417efc70(member, current ? current->Name_at_34 : Name{0}, mode);
            FreeString(mode);
        }
    }
    if (Analytics* analytics = GetAnalytics_14181af10()) {
        // The observed body dereferences selected here even when null.
        FString* mode = StringFromField_141f3b040(selected->Field_at_3c);
        Playlist* current = self->Current();
        SetAnalytics(analytics, current ? current->Name_at_34 : Name{0}, mode);
        FreeString(mode);
    }
    selected = self->Current();
    if (selected) {
        if (self->LocalRole == 3) self->Field_at_300 = selected->Field_at_58;
        self->TotalFinalCountdownTime = int32_t(EvaluatePlaylist_14070fe00(selected, 0, 0));
    }
    Call_14199f770(self);
}
