from pathlib import Path
import subprocess,json,tempfile
HERE=Path(__file__).resolve().parent
work=tempfile.TemporaryDirectory(prefix='common-button-callbacks-')
w=Path(work.name)
s=(HERE/'button-definitions.inc').read_text();s=s[s.index('bool UCommonButtonInternal::IsCommonInputSuspended'):]
prefix=r'''
#include <cassert>
#include <functional>
#include <vector>
struct FReply { bool handled; int marker; static FReply Handled(){return {true,0};} static FReply Unhandled(){return {false,0};} };
struct UCommonInputManager { bool fd=false, fe=false; bool IsInputSuspended()const{return fd||fe;} };
struct UCommonUISubsystem { UCommonInputManager* CommonInputManager=nullptr; };
struct UGameInstance { UCommonUISubsystem* sub=nullptr;template<class T>T* GetSubsystem(){return sub;} };
struct UWorld { UGameInstance* gi=nullptr;UGameInstance* GetGameInstance(){return gi;} };
struct UButton { int clicks=0,presses=0,releases=0,setEnabledCalls=0;void SetIsEnabled(bool){++setEnabledCalls;}FReply SlateHandleClicked(){++clicks;return {true,42};}void SlateHandlePressed(){++presses;}void SlateHandleReleased(){++releases;} };
struct ReplyDelegate { std::function<FReply()> fn;bool safe=true;bool IsBound(){return bool(fn)&&safe;}FReply Execute(){return fn();} };
struct VoidDelegate { std::function<void()> fn;bool safe=true;bool IsBound(){return bool(fn)&&safe;}void Broadcast(){if(IsBound())fn();}void ExecuteIfBound(){if(IsBound())fn();} };
struct SCommonButton { bool bButtonEnabled=true;bool enabled=true;int calls=0;void SetInteractionEnabled(bool b){enabled=b;++calls;} };
struct ButtonPtr { SCommonButton* p=nullptr;SCommonButton* Get(){return p;}bool IsValid()const{return p!=nullptr;}SCommonButton* operator->(){return p;} };
struct UCommonButtonInternal : UButton {
 UWorld* world=nullptr;UWorld* GetWorld(){return world;}bool bInteractionEnabled=true,bButtonEnabled=true;ButtonPtr NativeCommonButton;
 ReplyDelegate NativeDoubleClickDelegate;VoidDelegate OnDoubleClicked,NativeFocusReceivedDelegate;
 bool IsCommonInputSuspended();FReply SlateHandleCommonClicked();FReply SlateHandleCommonDoubleClicked();void SlateHandleCommonPressed();void SlateHandleCommonReleased();void SlateHandleCommonFocusReceived();void NativeSetInteractionEnabled(bool);void NativeSetButtonEnabled(bool);
};
'''
tests=r'''
int main(){
 int rows=0;
 for(int stage=0;stage<5;++stage)for(int flags=0;flags<4;++flags){
 UCommonButtonInternal b;UWorld w;UGameInstance gi;UCommonUISubsystem sub;UCommonInputManager mgr;
 mgr.fd=flags&1;mgr.fe=flags&2;if(stage>=1)b.world=&w;if(stage>=2)w.gi=&gi;if(stage>=3)gi.sub=&sub;if(stage>=4)sub.CommonInputManager=&mgr;
 bool blocked=stage==4&&flags;
 FReply click=b.SlateHandleCommonClicked();b.SlateHandleCommonPressed();b.SlateHandleCommonReleased();
 assert(click.handled&&click.marker==(blocked?0:42));assert(b.clicks==!blocked&&b.presses==!blocked&&b.releases==!blocked);
 for(int native=0;native<4;++native)for(int dynamic=0;dynamic<2;++dynamic){
 std::vector<int> order;b.NativeDoubleClickDelegate.fn=nullptr;b.OnDoubleClicked.fn=nullptr;b.NativeDoubleClickDelegate.safe=native!=3;
 if(native)b.NativeDoubleClickDelegate.fn=[&](){order.push_back(1);return FReply{native==2,99};};
 if(dynamic)b.OnDoubleClicked.fn=[&](){order.push_back(2);};
 FReply reply=b.SlateHandleCommonDoubleClicked();
 if(blocked){assert(!reply.handled&&reply.marker==0);assert(order.empty());}
 else{assert(reply.handled==bool(dynamic||native==2));assert(reply.marker==(dynamic?0:(native&&native!=3)?99:0));std::vector<int> expected;if(native&&native!=3)expected.push_back(1);if(dynamic)expected.push_back(2);assert(order==expected);}
 ++rows;
 }
 // Focus callback deliberately has no input suspension guard.
 int focus=0;b.NativeFocusReceivedDelegate.fn=nullptr;b.SlateHandleCommonFocusReceived();
 b.NativeFocusReceivedDelegate.fn=[&](){++focus;};b.NativeFocusReceivedDelegate.safe=false;b.SlateHandleCommonFocusReceived();assert(!focus);
 b.NativeFocusReceivedDelegate.safe=true;b.SlateHandleCommonFocusReceived();assert(focus==1);
 }
 assert(rows==160);
 for(int present=0;present<2;++present){UCommonButtonInternal b;SCommonButton sb;b.NativeCommonButton.p=present?&sb:nullptr;
 b.NativeSetInteractionEnabled(true);assert(sb.calls==0&&b.bInteractionEnabled);
 b.NativeSetInteractionEnabled(false);assert(!b.bInteractionEnabled&&sb.calls==present);if(present)assert(!sb.enabled);
 b.NativeSetInteractionEnabled(false);assert(sb.calls==present);
 b.NativeSetInteractionEnabled(true);assert(b.bInteractionEnabled&&sb.calls==2*present);if(present)assert(sb.enabled);
 }
 for(int present=0;present<2;++present){UCommonButtonInternal b;SCommonButton sb;b.NativeCommonButton.p=present?&sb:nullptr;
 int callbacks=0;b.NativeDoubleClickDelegate.fn=[&](){++callbacks;return FReply::Handled();};
 b.OnDoubleClicked.fn=[&](){++callbacks;};b.NativeFocusReceivedDelegate.fn=[&](){++callbacks;};
 for(bool value : {false,true,false,true}){
 sb.bButtonEnabled=!value;b.NativeSetButtonEnabled(value);
 assert(b.bButtonEnabled==value);if(present)assert(sb.bButtonEnabled==value);else assert(sb.bButtonEnabled!=value);
 assert(b.bInteractionEnabled&&sb.enabled&&sb.calls==0);
 assert(b.setEnabledCalls==0&&b.clicks==0&&b.presses==0&&b.releases==0&&callbacks==0);
 }
 // Repeated values still synchronize a live Slate object whose flag changed independently.
 if(present){sb.bButtonEnabled=false;b.NativeSetButtonEnabled(true);assert(sb.bButtonEnabled);}
 assert(b.setEnabledCalls==0&&sb.calls==0&&callbacks==0);
 }
}
'''
(w/'callbacks-control.cpp').write_text(prefix+s+tests)
cmd=['c++','-std=c++14','-Wall','-Wextra','-Werror','-fsanitize=address,undefined','-fno-omit-frame-pointer',str(w/'callbacks-control.cpp'),'-o',str(w/'callbacks-control')]
subprocess.run(cmd,check=True,cwd=w);subprocess.run([str(w/'callbacks-control')],check=True,cwd=w)
print('PASS combined exact callback body controls')
