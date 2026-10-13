#include "SceTypes.hpp"
#include "prx/libc/include/general/VabiMacros.hpp"

#include <cstdint>
#include <cstdlib>
#include <stdexcept>

extern "C" {
int APS5_VABI sceContentDeleteInitialize(const ContentDeleteInitParam* initParam);
int APS5_VABI sceContentDeleteTerminate(void);
int APS5_VABI sceContentDeleteById(std::int64_t contentId);
int APS5_VABI sceContentDeleteByPath(const char* contentPath);
}

namespace {

void Require(bool value) { if (!value) std::abort(); }

template <typename TAction>
bool ThrowsLogic(TAction action) {
    try {
        action();
    } catch (const std::logic_error&) {
        return true;
    }
    return false;
}

}

int main() {
    constexpr int invalidParam = static_cast<int>(0x809D5001u);
    constexpr int noPermission = static_cast<int>(0x809D5006u);

    Require(ThrowsLogic([] { sceContentDeleteById(1); }));
    Require(ThrowsLogic([] { sceContentDeleteByPath("/user/data/capture.jpg"); }));

    ContentDeleteInitParam init{};
    init.heap_size = 16 * 1024;
    Require(sceContentDeleteInitialize(&init) == 0);

    Require(sceContentDeleteByPath(nullptr) == invalidParam);
    Require(sceContentDeleteByPath("") == invalidParam);
    Require(sceContentDeleteByPath("/user/data/capture.jpg") == noPermission);
    Require(sceContentDeleteById(0) == noPermission);
    Require(sceContentDeleteById(0x123456789abcdefLL) == noPermission);

    Require(sceContentDeleteTerminate() == 0);
    Require(ThrowsLogic([] { sceContentDeleteById(1); }));
    Require(ThrowsLogic([] { sceContentDeleteByPath("/user/data/capture.jpg"); }));
}
