#include "prx/libc/include/general/VabiMacros.hpp"
#include <cstdint>
#include <cstdlib>
#include <array>
#include <stdexcept>

extern "C" {
int APS5_VABI sceWebBrowserDialogInitialize(void);
int APS5_VABI sceWebBrowserDialogTerminate(void);
int APS5_VABI sceWebBrowserDialogOpen(const void* param);
int APS5_VABI sceWebBrowserDialogClose(void);
int APS5_VABI sceWebBrowserDialogGetStatus(void);
int APS5_VABI sceWebBrowserDialogUpdateStatus(void);
int APS5_VABI sceWebBrowserDialogGetResult(void* result);

struct WebBrowserDialogPredeterminedContentParam {
    std::uint64_t size;
    std::array<const char*, 20> domains;
    std::array<std::uint8_t, 256> reserved;
};
struct WebBrowserDialogResetCookieParam {
    std::uint64_t size;
    std::array<std::uint8_t, 256> reserved;
};
struct WebBrowserDialogSetCookieParam {
    std::uint64_t size;
    const char* url;
    const char* cookie;
    std::array<std::uint8_t, 256> reserved;
};
int APS5_VABI sceWebBrowserDialogOpenForPredeterminedContent(const void*, const WebBrowserDialogPredeterminedContentParam*);
int APS5_VABI sceWebBrowserDialogResetCookie(const WebBrowserDialogResetCookieParam*);
int APS5_VABI sceWebBrowserDialogSetCookie(const WebBrowserDialogSetCookieParam*);
}

static_assert(sizeof(WebBrowserDialogPredeterminedContentParam) == 424);
static_assert(sizeof(WebBrowserDialogResetCookieParam) == 264);
static_assert(sizeof(WebBrowserDialogSetCookieParam) == 280);

namespace {

constexpr int COMMON_DIALOG_STATUS_NONE = 0;
constexpr int COMMON_DIALOG_STATUS_INITIALIZED = 1;
constexpr int COMMON_DIALOG_STATUS_FINISHED = 3;
constexpr int COMMON_DIALOG_RESULT_USER_CANCELED = 1;
constexpr int COMMON_DIALOG_ERROR_NOT_INITIALIZED = static_cast<int>(0x80B80003u);
constexpr int COMMON_DIALOG_ERROR_NOT_FINISHED = static_cast<int>(0x80B80005u);
constexpr int COMMON_DIALOG_ERROR_ARG_NULL = static_cast<int>(0x80B8000Du);

void Require(bool value) { if (!value) std::abort(); }

template <typename Call>
bool ThrowsInvalidArgument(Call call) {
    try {
        call();
    } catch (const std::invalid_argument&) {
        return true;
    }
    return false;
}

}

int main() {
    std::uint8_t param[64] = {};
    std::int32_t result = 0x5a5a5a5a;

    Require(sceWebBrowserDialogGetStatus() == COMMON_DIALOG_STATUS_NONE);
    Require(sceWebBrowserDialogClose() == COMMON_DIALOG_ERROR_NOT_INITIALIZED);
    Require(sceWebBrowserDialogOpen(param) == COMMON_DIALOG_ERROR_NOT_INITIALIZED);
    Require(sceWebBrowserDialogGetResult(&result) == COMMON_DIALOG_ERROR_NOT_INITIALIZED);
    Require(result == 0x5a5a5a5a);
    WebBrowserDialogSetCookieParam cookie{};
    cookie.size = sizeof(cookie);
    cookie.url = "https://example.com";
    cookie.cookie = "session=abc";
    WebBrowserDialogResetCookieParam reset{};
    reset.size = sizeof(reset);
    WebBrowserDialogPredeterminedContentParam predetermined{};
    predetermined.size = sizeof(predetermined);
    predetermined.domains[0] = "example.com";
    Require(sceWebBrowserDialogSetCookie(&cookie) == COMMON_DIALOG_ERROR_NOT_INITIALIZED);
    Require(sceWebBrowserDialogResetCookie(&reset) == COMMON_DIALOG_ERROR_NOT_INITIALIZED);
    Require(sceWebBrowserDialogOpenForPredeterminedContent(param, &predetermined) == COMMON_DIALOG_ERROR_NOT_INITIALIZED);

    Require(sceWebBrowserDialogInitialize() == 0);
    Require(sceWebBrowserDialogGetStatus() == COMMON_DIALOG_STATUS_INITIALIZED);
    Require(sceWebBrowserDialogUpdateStatus() == COMMON_DIALOG_STATUS_INITIALIZED);
    Require(sceWebBrowserDialogGetResult(&result) == COMMON_DIALOG_ERROR_NOT_FINISHED);
    Require(result == 0x5a5a5a5a);
    Require(sceWebBrowserDialogSetCookie(nullptr) == COMMON_DIALOG_ERROR_ARG_NULL);
    Require(sceWebBrowserDialogResetCookie(nullptr) == COMMON_DIALOG_ERROR_ARG_NULL);
    Require(sceWebBrowserDialogSetCookie(&cookie) == 0);
    WebBrowserDialogSetCookieParam cookieNoUrl = cookie;
    cookieNoUrl.url = nullptr;
    Require(sceWebBrowserDialogSetCookie(&cookieNoUrl) == COMMON_DIALOG_ERROR_ARG_NULL);
    WebBrowserDialogSetCookieParam badCookie = cookie;
    badCookie.size = 0;
    Require(ThrowsInvalidArgument([&] { sceWebBrowserDialogSetCookie(&badCookie); }));
    Require(sceWebBrowserDialogResetCookie(&reset) == 0);
    WebBrowserDialogResetCookieParam badReset = reset;
    badReset.size = 0;
    Require(ThrowsInvalidArgument([&] { sceWebBrowserDialogResetCookie(&badReset); }));

    Require(sceWebBrowserDialogTerminate() == 0);
    Require(sceWebBrowserDialogGetStatus() == COMMON_DIALOG_STATUS_NONE);

    Require(sceWebBrowserDialogInitialize() == 0);
    Require(sceWebBrowserDialogOpen(nullptr) == COMMON_DIALOG_ERROR_ARG_NULL);
    Require(sceWebBrowserDialogOpenForPredeterminedContent(nullptr, &predetermined) == COMMON_DIALOG_ERROR_ARG_NULL);
    Require(sceWebBrowserDialogOpenForPredeterminedContent(param, nullptr) == COMMON_DIALOG_ERROR_ARG_NULL);
    WebBrowserDialogPredeterminedContentParam badPredetermined = predetermined;
    badPredetermined.size = 0;
    Require(ThrowsInvalidArgument([&] { sceWebBrowserDialogOpenForPredeterminedContent(param, &badPredetermined); }));
    Require(sceWebBrowserDialogOpenForPredeterminedContent(param, &predetermined) == 0);
    Require(sceWebBrowserDialogGetStatus() == COMMON_DIALOG_STATUS_FINISHED);
    Require(sceWebBrowserDialogUpdateStatus() == COMMON_DIALOG_STATUS_FINISHED);

    Require(sceWebBrowserDialogGetResult(nullptr) == COMMON_DIALOG_ERROR_ARG_NULL);
    Require(sceWebBrowserDialogGetResult(&result) == 0);
    Require(result == COMMON_DIALOG_RESULT_USER_CANCELED);

    Require(sceWebBrowserDialogClose() == 0);
    Require(sceWebBrowserDialogTerminate() == 0);
    Require(sceWebBrowserDialogGetStatus() == COMMON_DIALOG_STATUS_NONE);

    return 0;
}
