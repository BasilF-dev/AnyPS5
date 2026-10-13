#include <cstdint>
#include <cstddef>
#include "SceTypes.hpp"
#include "prx/libc/include/General.hpp"
#include <atomic>
#include <stdexcept>
#include <string>

namespace {
constexpr int ContentDeleteErrorInvalidParam = static_cast<int>(0x809D5001u);
constexpr int ContentDeleteErrorNoPermission = static_cast<int>(0x809D5006u);
std::atomic<bool> g_initialized{false};

void RequireInitialized(const char* function) {
    if (!g_initialized.load()) throw std::logic_error(std::string(function) + ": not initialized");
}
}

extern "C" {

int APS5_VABI sceContentDeleteInitialize(const ContentDeleteInitParam* init_param) {
    if (init_param == nullptr) APS5_INVALID_ARG_EX;
    bool expected = false;
    if (!g_initialized.compare_exchange_strong(expected, true)) throw std::logic_error(std::string(__func__) + ": already initialized");
    return 0;
}

int APS5_VABI sceContentDeleteByPath(const char* contentPath) {
    RequireInitialized(__func__);
    if (contentPath == nullptr || *contentPath == '\0') return ContentDeleteErrorInvalidParam;
    return ContentDeleteErrorNoPermission;
}

int APS5_VABI sceContentDeleteById(std::int64_t contentId) {
    RequireInitialized(__func__);
    (void)contentId;
    return ContentDeleteErrorNoPermission;
}

int APS5_VABI sceContentDeleteTerminate(void) {
    bool expected = true;
    if (!g_initialized.compare_exchange_strong(expected, false)) throw std::logic_error(std::string(__func__) + ": not initialized");
    return 0;
}

}
