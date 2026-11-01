import subprocess
import tempfile
import unittest
from pathlib import Path

HERE = Path(__file__).resolve().parent

# Portable collaborators exercise the actual checked-in definitions, without UE headers.
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
struct FCsvProfiler { static void RecordEventf(int category, const char* format, const char* name) { assert(category==17); assert(std::string(format)=="Activate: %s"); assert(std::string(name)=="TestPanel"); assert(intro_flag && !*intro_flag); assert(name_alive); ++csv_events; } };

struct FDelegateHandle { int id=0; };
template<class T> struct TWeakObjectPtr { T* object=nullptr; T* Get(){trace.push_back("resolve-manager");return object;} };
struct FCommonPanelOperationCallback {std::function<void(UCommonActivatablePanel*)> cb;void ExecuteIfBound(UCommonActivatablePanel* p){if(cb)cb(p);} };
struct MethodDelegate {std::function<void(ECommonInputType)> cb;template<class T>FDelegateHandle AddUObject(T* o,void(T::*f)(ECommonInputType)){trace.push_back("bind-input");cb=[=](ECommonInputType t){(o->*f)(t);};return {1};} };
struct UCommonInputSubsystem { MethodDelegate NativeInputMethodChanged; };
struct UCommonGlobalInputHandler {uint32 NativePriorityFilter=99;};
struct UCommonInputManager { UCommonGlobalInputHandler* GlobalInputHandler=nullptr;};
struct UCommonUISubsystem {UCommonInputManager* CommonInputManager=nullptr;};
struct ULocalPlayer {UCommonInputSubsystem* input=nullptr;template<class T>T*GetSubsystem(){trace.push_back("input-subsystem");return input;}};
struct UGameInstance {UCommonUISubsystem* ui=nullptr;template<class T>T*GetSubsystem(){trace.push_back("ui-subsystem");return ui;}};
struct PanelDynamic {std::function<void(UCommonActivatablePanel*)> cb;void Broadcast(UCommonActivatablePanel* p){trace.push_back("panel-broadcast");if(cb)cb(p);}};
struct UCommonActivatableWidget {bool bIsActive=false;virtual void NativeActivateRequest(){bIsActive=true;NativeHandleActivated();} void ActivateWidget(){if(!bIsActive)NativeActivateRequest();} virtual void NativeHandleActivated(){if(bIsActive){trace.push_back("base-notify");}}};
struct UCommonActivatablePanel: UCommonActivatableWidget {
 using Super=UCommonActivatableWidget;
 ULocalPlayer* player=nullptr;UGameInstance* instance=nullptr;
 PanelDynamic OnWidgetActivated;
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

struct Derived:UCommonActivatablePanel {
 std::function<void()> complete;
 void NativeIntroComplete()override{trace.push_back("intro-hook");if(complete)complete();}
 void NativeInputMethodChangedHook(ECommonInputType)override{trace.push_back("input-hook");}
 uint32 NativeGetGlobalInputPriorityFilter()const override{trace.push_back("filter-hook");return 7;}
};
int main(){
 // Ordinary ActivateWidget enters the Panel intro override exactly once.
 Derived requested;trace.clear();requested.ActivateWidget();
 assert(requested.bNativeIsIntroed&&requested.bIsActive);
#if CSV_PROFILER
 assert(csv_events==1);
#else
 assert(csv_events==0);
#endif
 assert((trace==std::vector<std::string>{"intro-event","intro-hook","player","resolve-manager","base-notify","panel-broadcast"}));
 trace.clear();requested.NativeActivateRequest();assert(trace.empty());
#if CSV_PROFILER
 assert(csv_events==1);
#endif
 // Pending BP intro can reenter before EndIntro: flag is already true.
 Derived pending;pending.blueprintIntro=[&]{pending.ActivateWidget();};trace.clear();pending.ActivateWidget();
 assert(pending.bNativeIsIntroed&&!pending.bIsActive);
 assert((trace==std::vector<std::string>{"intro-event"}));
 trace.clear();pending.ActivateWidget();assert(trace.empty());
 pending.EndIntro();assert(pending.bIsActive);trace.clear();
 UCommonGlobalInputHandler global;UCommonInputManager manager;manager.GlobalInputHandler=&global;
 UCommonUISubsystem ui;ui.CommonInputManager=&manager;UGameInstance gi;gi.ui=&ui;
 UCommonInputSubsystem input;ULocalPlayer player;player.input=&input;
 Derived p;p.player=&player;p.instance=&gi;p.NativeInputManager.object=&manager;
 p.NativePreActivation.cb=[](UCommonActivatablePanel*){trace.push_back("pre");};
 p.NativePostActivation.cb=[](UCommonActivatablePanel*){trace.push_back("post");};
 p.OnBeginIntro_Implementation();assert(p.bIsActive&&global.NativePriorityFilter==7&&p.NativeInputMethodHandle.id==1);
 assert((trace==std::vector<std::string>{"intro-hook","player","input-subsystem","game-instance","ui-subsystem","filter-hook","bind-input","pre","resolve-manager","base-notify","panel-broadcast","post"}));
 trace.clear();p.EndIntro();assert((trace==std::vector<std::string>{"intro-hook"}));
 trace.clear();input.NativeInputMethodChanged.cb(ECommonInputType::Gamepad);
 assert((trace==std::vector<std::string>{"input-hook","BP-input-gamepad"}));
 trace.clear();input.NativeInputMethodChanged.cb(ECommonInputType::MouseAndKeyboard);
 assert((trace==std::vector<std::string>{"input-hook","BP-input-other"}));
 // Completion can set active synchronously, suppressing outer activation.
 Derived reentrant;reentrant.complete=[&]{reentrant.bIsActive=true;};trace.clear();reentrant.EndIntro();
 assert((trace==std::vector<std::string>{"intro-hook"}));
 // A pre callback can clear active; Panel broadcast/post still execute while base is guarded.
 p.bIsActive=false;p.NativePreActivation.cb=[](UCommonActivatablePanel*x){trace.push_back("pre-clear");x->bIsActive=false;};trace.clear();p.EndIntro();
 assert((trace==std::vector<std::string>{"intro-hook","player","input-subsystem","game-instance","ui-subsystem","filter-hook","bind-input","pre-clear","resolve-manager","panel-broadcast","post"}));
 Derived absent;trace.clear();absent.EndIntro();assert(absent.bIsActive);
 assert((trace==std::vector<std::string>{"intro-hook","player","resolve-manager","base-notify","panel-broadcast"}));
}
'''

class PanelIntroCore(unittest.TestCase):
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
