#include <initializer_list>
#include <cassert>
class UCommonInputManager {
public:
 bool bNativeOperationInputSuspended = false;
 bool bNativeExternalInputSuspended = false;
 bool IsInputSuspended() const;
 void NativeSetExternalInputSuspended(bool);
};
#include "manager-definitions.inc"
int main() {
 UCommonInputManager m;
 assert(!m.IsInputSuspended());
 for (bool operation : {false,true}) for (bool external : {false,true}) {
  m.bNativeOperationInputSuspended=operation;
  m.NativeSetExternalInputSuspended(external);
  assert(m.IsInputSuspended()==(operation || external));
  assert(m.bNativeOperationInputSuspended==operation);
 }
}
