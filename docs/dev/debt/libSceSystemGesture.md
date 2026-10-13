# libSceSystemGesture technical debt

### Silent stubs

- [System gesture recognizers](../../../core/libs/prx/libSceSystemGesture/Export.cpp) (libSceSystemGesture) - no touch input is emulated and no recognizer state is kept, so a title never receives a gesture event. The bulk/count and index/ID lookup calls therefore report an empty event stream; the ID lookups zero a supplied output before returning the library's existing out-of-range result. `sceSystemGestureGetTouchRecognizerInformation` validates its handle and pointers, then returns a zeroed information block because there is no stored recognizer state. This follows the no-input shape of KytyPS5's SystemGesture implementation while retaining AnyPS5's existing 0x80D1 error namespace; KytyPS5 uses 0x8089 errors instead. Console error codes and zeroing behavior are not verified on PS5 hardware.
