#include "SceTypes.hpp"
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <stdexcept>

struct NpWebApi2MemoryPoolStats {
    std::size_t pool_size;
    std::size_t max_inuse_size;
    std::size_t current_inuse_size;
    std::int32_t reserved;
};

extern "C" {
int APS5_VABI sceNpWebApi2Initialize(int, std::size_t);
int APS5_VABI sceNpWebApi2Terminate(int);
int APS5_VABI sceNpWebApi2GetMemoryPoolStats(int, NpWebApi2MemoryPoolStats*);
int APS5_VABI sceNpWebApi2PushEventCreateHandle(int);
int APS5_VABI sceNpWebApi2PushEventSetHandleTimeout(int, int, std::uint32_t);
int APS5_VABI sceNpWebApi2PushEventAbortHandle(int, int);
int APS5_VABI sceNpWebApi2PushEventDeleteHandle(int, int);
int APS5_VABI sceNpWebApi2CreateUserContext(int, int);
int APS5_VABI sceNpWebApi2CreateRequest(int, const char*, const char*, const char*, const void*, std::int64_t*);
int APS5_VABI sceNpWebApi2DeleteRequest(std::int64_t);
int APS5_VABI sceNpWebApi2AddWebTraceTag(std::int64_t, const char*);
int APS5_VABI sceNpWebApi2SetRequestTimeout(std::int64_t, std::uint32_t);
}

static void Require(bool value) { if (!value) std::abort(); }

template <typename TCall>
static bool Throws(TCall call) {
    try {
        call();
    } catch (const std::exception&) {
        return true;
    }
    return false;
}

int main() {
    const int context = sceNpWebApi2Initialize(1, 0x20000);
    const int other = sceNpWebApi2Initialize(1, 0x8000);
    Require(context > 0 && other > 0 && context != other);
    NpWebApi2MemoryPoolStats stats{1, 1, 1, 1};
    Require(sceNpWebApi2GetMemoryPoolStats(context, &stats) == 0);
    Require(stats.pool_size == 0x20000 && stats.max_inuse_size == 0 && stats.current_inuse_size == 0 && stats.reserved == 0);
    Require(sceNpWebApi2GetMemoryPoolStats(other, &stats) == 0 && stats.pool_size == 0x8000);
    Require(Throws([&] { sceNpWebApi2GetMemoryPoolStats(context, nullptr); }));
    constexpr int invalidArgument = static_cast<int>(0x80553402u);
    constexpr int libContextNotFound = static_cast<int>(0x80553404u);
    constexpr int requestNotFound = static_cast<int>(0x80553406u);
    constexpr int handleNotFound = static_cast<int>(0x8055340du);

    const int handle = sceNpWebApi2PushEventCreateHandle(context);
    Require(handle > 0);
    Require(sceNpWebApi2PushEventSetHandleTimeout(context, handle, 30000) == 0);
    Require(sceNpWebApi2PushEventAbortHandle(context, handle) == 0);
    Require(sceNpWebApi2PushEventAbortHandle(other, handle) == handleNotFound);
    Require(sceNpWebApi2PushEventAbortHandle(0x7fffffff, handle) == libContextNotFound);

    const int userContext = sceNpWebApi2CreateUserContext(context, 1);
    std::int64_t request = 0;
    Require(userContext > 0);
    Require(sceNpWebApi2CreateRequest(userContext, "group", "/resource", "GET", nullptr, &request) == 0 && request > 0);
    Require(sceNpWebApi2AddWebTraceTag(request, "trace") == 0);
    Require(sceNpWebApi2AddWebTraceTag(request, nullptr) == invalidArgument);
    Require(sceNpWebApi2SetRequestTimeout(request, 15000) == 0);
    Require(sceNpWebApi2DeleteRequest(request) == 0);
    Require(sceNpWebApi2SetRequestTimeout(request, 1) == requestNotFound);

    Require(sceNpWebApi2PushEventDeleteHandle(context, handle) == 0);
    Require(sceNpWebApi2PushEventAbortHandle(context, handle) == handleNotFound);
    Require(sceNpWebApi2Terminate(context) == 0);
    Require(Throws([&] { sceNpWebApi2GetMemoryPoolStats(context, &stats); }));
    Require(sceNpWebApi2PushEventSetHandleTimeout(context, handle, 30000) == libContextNotFound);
    Require(sceNpWebApi2Terminate(other) == 0);
    return 0;
}
