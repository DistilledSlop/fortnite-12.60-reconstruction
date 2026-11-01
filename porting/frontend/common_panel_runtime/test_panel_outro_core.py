import subprocess,tempfile,unittest
from pathlib import Path
HERE=Path(__file__).resolve().parent
CONTROL = r'''


#include <cassert>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>
using uint32=uint32_t;
enum class ECommonInputType { MouseAndKeyboard, Gamepad, Touch };
std::vector<std::string> trace;
struct UCommonActivatablePanel;
int csv_events=0; bool* intro_flag=nullptr; bool name_alive=false;
struct FString { std::string value; explicit FString(std::string v): value(v) { name_alive=true; } ~FString() { name_alive=false; } const char* operator*() const { return value.c_str(); } };
struct FName { FString ToString() const { return FString("TestPanel"); } };
#define TEXT(x) x
#define CSV_DECLARE_CATEGORY_MODULE_EXTERN(api, name)
#define CSV_CATEGORY_INDEX(name) 17
struct FCsvProfiler { static void RecordEventf(int category, const char* format, const char* name) { assert(category==17); assert(std::string(format)=="Activate: %s" || std::string(format)=="Deactivate: %s"); assert(std::string(name)=="TestPanel"); if(std::string(format)=="Activate: %s")assert(intro_flag && !*intro_flag);else trace.push_back("csv-deactivate"); assert(name_alive); ++csv_events; } };

struct FDelegateHandle { int id=0; bool IsValid()const{return id!=0;} };
template<class T> struct TWeakObjectPtr { T* object=nullptr; bool IsValid(){trace.push_back("valid-manager");return object!=nullptr;} T* Get(){trace.push_back("resolve-manager");return object;} };
struct FCommonPanelOperationCallback {std::function<void(UCommonActivatablePanel*)> cb;void ExecuteIfBound(UCommonActivatablePanel* p){if(cb)cb(p);} };
struct MethodDelegate {void Remove(FDelegateHandle h){assert(h.id==1);trace.push_back("remove-input");cb={};}std::function<void(ECommonInputType)> cb;template<class T>FDelegateHandle AddUObject(T* o,void(T::*f)(ECommonInputType)){trace.push_back("bind-input");cb=[=](ECommonInputType t){(o->*f)(t);};return {1};} };
struct UCommonInputSubsystem { MethodDelegate NativeInputMethodChanged; };
struct UCommonGlobalInputHandler {uint32 NativePriorityFilter=99;};
struct HeldInterface {void* object=reinterpret_cast<void*>(1);void* iface=reinterpret_cast<void*>(2);void SetObject(void*p){trace.push_back("clear-object");object=p;}void SetInterface(void*p){trace.push_back("clear-interface");iface=p;}};
struct UCommonInputManager {HeldInterface CurrentlyHeldActionInputHandler; UCommonGlobalInputHandler* GlobalInputHandler=nullptr;};
struct UCommonUISubsystem {UCommonInputManager* CommonInputManager=nullptr;};
struct ULocalPlayer {UCommonInputSubsystem* input=nullptr;template<class T>T*GetSubsystem(){trace.push_back("input-subsystem");return input;}};
struct UGameInstance {UCommonUISubsystem* ui=nullptr;template<class T>T*GetSubsystem(){trace.push_back("ui-subsystem");return ui;}};
struct PanelDynamic {std::function<void(UCommonActivatablePanel*)> cb;void Broadcast(UCommonActivatablePanel* p){trace.push_back("panel-broadcast");if(cb)cb(p);}};
struct FDataTableRowHandle{void* DataTable=nullptr;int RowName=0;};constexpr int NAME_None=0;
struct Progress {std::function<void(float)> cb;void Broadcast(float v){assert(v==0);trace.push_back("zero-progress");if(cb)cb(v);}};
struct Record{Progress NativeProgress;};
struct FCommonPanelActionMap{Record record;bool found=true;Record*Find(const FDataTableRowHandle&){return found?&record:nullptr;}};
struct UCommonActivatableWidget {virtual void NativeDeactivateRequest(){bIsActive=false;NativeHandleDeactivated();} void DeactivateWidget(){if(bIsActive)NativeDeactivateRequest();} virtual void NativeHandleDeactivated(){if(!bIsActive)trace.push_back("base-deactivate");}bool bIsActive=false;virtual void NativeActivateRequest(){bIsActive=true;NativeHandleActivated();} void ActivateWidget(){if(!bIsActive)NativeActivateRequest();} virtual void NativeHandleActivated(){if(bIsActive){trace.push_back("base-notify");}}};
struct UCommonActivatablePanel: UCommonActivatableWidget {
 using Super=UCommonActivatableWidget;
 ULocalPlayer* player=nullptr;UGameInstance* instance=nullptr;
 PanelDynamic OnWidgetActivated,OnWidgetDeactivated;
 std::function<void()> blueprintOutro;
 void OnBeginOutro(){trace.push_back("outro-event");if(blueprintOutro)blueprintOutro();else OnBeginOutro_Implementation();}
 void OnBeginOutro_Implementation();void EndOutro();void NativeDeactivateRequest()override;void NativeHandleDeactivated()override;virtual void NativeOutroComplete();void NativeClearHeldAction();
 FDataTableRowHandle NativeHeldAction;float NativeHeldActionDuration=0;FCommonPanelActionMap NativeActionRecords;FCommonPanelOperationCallback NativePreDeactivation,NativePostDeactivation;
 ULocalPlayer*GetOwningLocalPlayer(){trace.push_back("player");return player;}
 UGameInstance*GetGameInstance(){trace.push_back("game-instance");return instance;}
 std::function<void()> blueprintIntro;
 FName GetFName(){intro_flag=&bNativeIsIntroed;return {}; }
 void OnBeginIntro(){assert(!name_alive);trace.push_back("intro-event");if(blueprintIntro)blueprintIntro();else OnBeginIntro_Implementation();}
 void OnBeginIntro_Implementation();void EndIntro();
 void NativeActivateRequest()override;bool bNativeIsIntroed=false;
 void NativeHandleActivated()override;
 virtual void NativeIntroComplete();virtual void NativeInputMethodChangedHook(ECommonInputType);
 virtual uint32 NativeGetGlobalInputPriorityFilter()const;
 void NativeInputMethodChanged(ECommonInputType);
 TWeakObjectPtr<UCommonInputManager> NativeInputManager;FDelegateHandle NativeInputMethodHandle;
 FCommonPanelOperationCallback NativePreActivation,NativePostActivation;
 void OnInputModeChanged(bool gamepad){trace.push_back(gamepad?"BP-input-gamepad":"BP-input-other");}
};
#include "panel-intro-definitions.inc"


#include "panel-outro-definitions.inc"

struct Derived:UCommonActivatablePanel{
 std::function<void()> complete;
 void NativeOutroComplete()override{trace.push_back("outro-hook");assert(!bNativeIsIntroed);if(complete)complete();}
};
void expect(std::vector<std::string> expected){assert(trace==expected);}
int main(){
 // Actual intro binds listener; default outro removes it before pre/base/Panel/post.
 UCommonInputSubsystem input;ULocalPlayer player;player.input=&input;UCommonInputManager first,second;
 Derived p;p.player=&player;p.NativeInputManager.object=&first;
 p.NativePreDeactivation.cb=[](UCommonActivatablePanel*){trace.push_back("pre-deactivate");};
 p.NativePostDeactivation.cb=[](UCommonActivatablePanel*){trace.push_back("post-deactivate");};
 p.NativeActivateRequest();assert(input.NativeInputMethodChanged.cb);trace.clear();
 p.DeactivateWidget();assert(!p.bIsActive&&!p.bNativeIsIntroed&&!input.NativeInputMethodChanged.cb);
 auto full=std::vector<std::string>{"valid-manager","resolve-manager","clear-object","clear-interface","outro-event","outro-hook","player","input-subsystem","remove-input","valid-manager","resolve-manager","clear-object","clear-interface","pre-deactivate","base-deactivate","panel-broadcast","post-deactivate"};
#if CSV_PROFILER
 full.push_back("csv-deactivate");
#endif
 expect(full);trace.clear();p.DeactivateWidget();expect({});
 // Deferred Blueprint outro retains active state/listener until EndOutro.
 p.blueprintOutro=[]{};p.NativeActivateRequest();trace.clear();p.DeactivateWidget();
 assert(p.bIsActive&&p.bNativeIsIntroed&&input.NativeInputMethodChanged.cb);
 auto deferred=std::vector<std::string>{"valid-manager","resolve-manager","clear-object","clear-interface","outro-event"};
#if CSV_PROFILER
 deferred.push_back("csv-deactivate");
#endif
 expect(deferred);trace.clear();p.EndOutro();assert(!p.bIsActive&&!p.bNativeIsIntroed&&!input.NativeInputMethodChanged.cb);
 // EndOutro always notifies, even twice; completion can synchronously activate.
 int posts=0;p.NativePostDeactivation.cb=[&](UCommonActivatablePanel*){++posts;};
 p.complete=[&]{p.NativeActivateRequest();};trace.clear();p.EndOutro();assert(!p.bIsActive&&p.bNativeIsIntroed&&posts==1);p.complete={};p.EndOutro();assert(posts==2);
 // Pre-deactivation reactivation suppresses guarded base notification, retaining Panel/post.
 Derived pre;pre.NativePreDeactivation.cb=[](UCommonActivatablePanel*x){trace.push_back("pre-reactivate");x->bIsActive=true;};pre.NativePostDeactivation.cb=[](UCommonActivatablePanel*){trace.push_back("post-reactivate");};trace.clear();pre.EndOutro();assert(pre.bIsActive);expect({"outro-hook","player","valid-manager","pre-reactivate","panel-broadcast","post-reactivate"});
 // Actual original completion leaf returns; request guard is independent of active flag.
 UCommonActivatablePanel leaf;leaf.bIsActive=true;trace.clear();leaf.NativeDeactivateRequest();expect({});assert(leaf.bIsActive);leaf.EndOutro();expect({"player","valid-manager","base-deactivate","panel-broadcast"});assert(!leaf.bIsActive);
 // Invalid weak manager returns without changing held state or broadcasting.
 Derived h;h.NativeHeldAction={reinterpret_cast<void*>(1),3};h.NativeHeldActionDuration=2;trace.clear();h.NativeClearHeldAction();expect({"valid-manager"});assert(h.NativeHeldActionDuration==2&&h.NativeHeldAction.RowName==3);
 // Full row condition is DataTable OR RowName; duration must be positive, entry present.
 for(int row=0;row<4;++row)for(int dur=0;dur<4;++dur)for(bool found:{false,true}){
  h.NativeInputManager.object=&first;h.NativeHeldAction={row&1?reinterpret_cast<void*>(1):nullptr,row&2?3:0};h.NativeHeldActionDuration=dur==0?-1.f:dur==1?0.f:dur==2?1.f:__builtin_nanf("");h.NativeActionRecords.found=found;
  int calls=0;h.NativeActionRecords.record.NativeProgress.cb=[&](float){++calls;};trace.clear();h.NativeClearHeldAction();assert(calls==((row!=0&&dur>=2&&found)?1:0));assert(h.NativeHeldActionDuration==0&&h.NativeHeldAction.RowName==0&&!h.NativeHeldAction.DataTable);
 }
 // Progress callback may replace the weak manager: clear the newly resolved manager.
 first.CurrentlyHeldActionInputHandler.object=reinterpret_cast<void*>(1);second.CurrentlyHeldActionInputHandler.object=reinterpret_cast<void*>(2);
 h.NativeInputManager.object=&first;h.NativeHeldAction.RowName=2;h.NativeHeldActionDuration=1;h.NativeActionRecords.found=true;
 h.NativeActionRecords.record.NativeProgress.cb=[&](float){assert(h.NativeHeldActionDuration==1&&h.NativeHeldAction.RowName==2);h.NativeInputManager.object=&second;};trace.clear();h.NativeClearHeldAction();expect({"valid-manager","zero-progress","resolve-manager","clear-object","clear-interface"});assert(first.CurrentlyHeldActionInputHandler.object&& !second.CurrentlyHeldActionInputHandler.object);
 // Synchronous reentry after OnBeginOutro still records the outer CSV after callback.
 Derived re;re.bIsActive=true;re.bNativeIsIntroed=true;re.blueprintOutro=[&]{trace.push_back("callback-enter");re.EndOutro();re.NativeActivateRequest();trace.push_back("callback-exit");};trace.clear();re.DeactivateWidget();assert(re.bIsActive&&re.bNativeIsIntroed);
#if CSV_PROFILER
 assert(trace.back()=="csv-deactivate");
#else
 assert(trace.back()=="callback-exit");
#endif
}
'''
class PanelOutroCore(unittest.TestCase):
    def test_live_definitions_with_and_without_csv(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            for header in ("UCommonInputSubsystem.h", "UCommonInputManager.h", "UCommonGlobalInputHandler.h", "UCommonUISubsystem.h", "Engine/LocalPlayer.h", "Engine/GameInstance.h", "ProfilingDebugging/CsvProfiler.h"):
                target = root / header
                target.parent.mkdir(parents=True, exist_ok=True)
                target.write_text("")
            source = root / "control.cpp"
            source.write_text(CONTROL)
            for enabled in (0, 1):
                executable = root / f"control{enabled}"
                with self.subTest(csv=enabled):
                    subprocess.run(["c++", "-std=c++14", "-Wall", "-Wextra", "-Werror", f"-DCSV_PROFILER={enabled}", "-I", str(root), "-I", str(HERE), str(source), "-o", str(executable)], check=True)
                    subprocess.run([str(executable)], check=True)

if __name__ == '__main__': unittest.main()
