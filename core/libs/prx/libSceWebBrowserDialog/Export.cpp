#include <atomic>
#include <cstdint>
#include <stdexcept>
#include <cstddef>
#include <array>
#include <mutex>
#include <string>
#include <utility>
#include <vector>
#include "SceTypes.hpp"
#include "prx/libc/include/General.hpp"

namespace {
std::atomic<int> g_status{0};
std::mutex g_sessionMutex;
std::vector<std::pair<std::string, std::string>> g_cookies;
std::vector<std::string> g_predeterminedDomains;

struct WebBrowserDialogPredeterminedContentParam {
    std::uint64_t size;
    std::array<const char*, 20> domains;
    std::array<std::uint8_t, 256> reserved;
};
static_assert(sizeof(WebBrowserDialogPredeterminedContentParam) == 424);

struct WebBrowserDialogResetCookieParam {
    std::uint64_t size;
    std::array<std::uint8_t, 256> reserved;
};
static_assert(sizeof(WebBrowserDialogResetCookieParam) == 264);

struct WebBrowserDialogSetCookieParam {
    std::uint64_t size;
    const char* url;
    const char* cookie;
    std::array<std::uint8_t, 256> reserved;
};
static_assert(sizeof(WebBrowserDialogSetCookieParam) == 280);

constexpr int COMMON_DIALOG_STATUS_NONE = 0;
constexpr int COMMON_DIALOG_STATUS_INITIALIZED = 1;
constexpr int COMMON_DIALOG_STATUS_RUNNING = 2;
constexpr int COMMON_DIALOG_STATUS_FINISHED = 3;
constexpr int COMMON_DIALOG_RESULT_USER_CANCELED = 1;
constexpr int COMMON_DIALOG_ERROR_NOT_INITIALIZED = static_cast<int>(0x80B80003u);
constexpr int COMMON_DIALOG_ERROR_NOT_FINISHED = static_cast<int>(0x80B80005u);
constexpr int COMMON_DIALOG_ERROR_BUSY = static_cast<int>(0x80B80007u);
constexpr int COMMON_DIALOG_ERROR_ARG_NULL = static_cast<int>(0x80B8000Du);
}

extern "C" {

int APS5_VABI sceWebBrowserDialogInitialize(void) {
    int expected = COMMON_DIALOG_STATUS_NONE;
    if (!g_status.compare_exchange_strong(expected, COMMON_DIALOG_STATUS_INITIALIZED))
        throw std::logic_error("sceWebBrowserDialogInitialize: already initialized");
    std::lock_guard lock(g_sessionMutex);
    g_cookies.clear();
    g_predeterminedDomains.clear();
    return 0;
}

int APS5_VABI sceWebBrowserDialogTerminate(void) {
    int expected = COMMON_DIALOG_STATUS_INITIALIZED;
    if (!g_status.compare_exchange_strong(expected, COMMON_DIALOG_STATUS_NONE)) {
        expected = COMMON_DIALOG_STATUS_FINISHED;
        if (!g_status.compare_exchange_strong(expected, COMMON_DIALOG_STATUS_NONE))
            throw std::logic_error("sceWebBrowserDialogTerminate: not initialized or still running");
    }
    std::lock_guard lock(g_sessionMutex);
    g_cookies.clear();
    g_predeterminedDomains.clear();
    return 0;
}

int APS5_VABI sceWebBrowserDialogClose(void) {
 if (g_status.load() == COMMON_DIALOG_STATUS_NONE) return COMMON_DIALOG_ERROR_NOT_INITIALIZED;
 return 0;
}

int APS5_VABI sceWebBrowserDialogGetResult(void* result) {
 const int status = g_status.load();
 if (status == COMMON_DIALOG_STATUS_NONE) return COMMON_DIALOG_ERROR_NOT_INITIALIZED;
 if (result == nullptr) return COMMON_DIALOG_ERROR_ARG_NULL;
 if (status != COMMON_DIALOG_STATUS_FINISHED) return COMMON_DIALOG_ERROR_NOT_FINISHED;
 *static_cast<std::int32_t*>(result) = COMMON_DIALOG_RESULT_USER_CANCELED;
 return 0;
}

int APS5_VABI sceWebBrowserDialogGetStatus(void) {
    return g_status.load();
}

int APS5_VABI sceWebBrowserDialogOpen(const void* param) {
 const int status = g_status.load();
 if (status == COMMON_DIALOG_STATUS_NONE) return COMMON_DIALOG_ERROR_NOT_INITIALIZED;
 if (status == COMMON_DIALOG_STATUS_RUNNING) return COMMON_DIALOG_ERROR_BUSY;
 if (param == nullptr) return COMMON_DIALOG_ERROR_ARG_NULL;
 g_status = COMMON_DIALOG_STATUS_FINISHED;
 return 0;
}

int APS5_VABI sceWebBrowserDialogUpdateStatus(void) {
    return g_status.load();
}


int APS5_VABI sceWebBrowserDialogSetCookie(const WebBrowserDialogSetCookieParam* param) {
    if (g_status.load() == COMMON_DIALOG_STATUS_NONE) return COMMON_DIALOG_ERROR_NOT_INITIALIZED;
    if (param == nullptr) return COMMON_DIALOG_ERROR_ARG_NULL;
    if (param->size != sizeof(*param)) throw std::invalid_argument("sceWebBrowserDialogSetCookie: invalid parameter size");
    if (param->url == nullptr || param->cookie == nullptr) return COMMON_DIALOG_ERROR_ARG_NULL;
    std::lock_guard lock(g_sessionMutex);
    g_cookies.emplace_back(param->url, param->cookie);
    return 0;
}

int APS5_VABI sceWebBrowserDialogOpenForPredeterminedContent(
    const void* param, const WebBrowserDialogPredeterminedContentParam* predetermined) {
    const int status = g_status.load();
    if (status == COMMON_DIALOG_STATUS_NONE) return COMMON_DIALOG_ERROR_NOT_INITIALIZED;
    if (status == COMMON_DIALOG_STATUS_RUNNING) return COMMON_DIALOG_ERROR_BUSY;
    if (param == nullptr || predetermined == nullptr) return COMMON_DIALOG_ERROR_ARG_NULL;
    if (predetermined->size != sizeof(*predetermined))
        throw std::invalid_argument("sceWebBrowserDialogOpenForPredeterminedContent: invalid parameter size");
    {
        std::lock_guard lock(g_sessionMutex);
        g_predeterminedDomains.clear();
        for (const char* domain : predetermined->domains) {
            if (domain != nullptr) g_predeterminedDomains.emplace_back(domain);
        }
    }
    g_status = COMMON_DIALOG_STATUS_FINISHED;
    return 0;
}

int APS5_VABI sceWebBrowserDialogResetCookie(const WebBrowserDialogResetCookieParam* param) {
    if (g_status.load() == COMMON_DIALOG_STATUS_NONE) return COMMON_DIALOG_ERROR_NOT_INITIALIZED;
    if (param == nullptr) return COMMON_DIALOG_ERROR_ARG_NULL;
    if (param->size != sizeof(*param)) throw std::invalid_argument("sceWebBrowserDialogResetCookie: invalid parameter size");
    std::lock_guard lock(g_sessionMutex);
    g_cookies.clear();
    return 0;
}

}
