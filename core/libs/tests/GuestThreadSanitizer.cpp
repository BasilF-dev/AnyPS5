#include "prx/libc/include/general/VabiMacros.hpp"
#include <cstdlib>

extern "C" {
int APS5_VABI sceKernelIsThreadSanitizerEnabled(void);
void APS5_VABI __tsan_acquire(void*);
void APS5_VABI __tsan_release(void*);
}

int main() {
    if (sceKernelIsThreadSanitizerEnabled() != 0) std::abort();
    int synchronizationPoint = 0;
    __tsan_release(&synchronizationPoint);
    __tsan_acquire(&synchronizationPoint);
    return 0;
}
