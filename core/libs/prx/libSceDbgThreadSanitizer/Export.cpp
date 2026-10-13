#include "SceTypes.hpp"
#include "prx/libc/include/General.hpp"

extern "C" {

void APS5_VABI __tsan_acquire(void* address) {
    (void)address;
}

void APS5_VABI __tsan_release(void* address) {
    (void)address;
}

}
