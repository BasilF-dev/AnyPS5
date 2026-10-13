#include <cstddef>
#include <cstdint>

#include "prx/libc/include/General.hpp"
#include "prx/libSceNgs2.native/include/Ngs2Types.hpp"
#include <memory>
#include <mutex>
#include <stdexcept>
#include <unordered_map>

#pragma GCC visibility push(default)

namespace {
struct ReportRegistration {
    uint32_t type;
    void* handler;
    uintptr_t user_data;
};

std::mutex g_reportMutex;
std::unordered_map<Ngs2Handle, std::unique_ptr<ReportRegistration>> g_reportHandlers;
}

extern "C" {

int APS5_VABI sceNgs2GeomApply(const Ngs2GeomListenerWork* listener, const Ngs2GeomSourceParam* source, Ngs2GeomAttribute* out_attrib, uint32_t flags) {
    (void)listener;
    (void)source;
    (void)out_attrib;
    (void)flags;
    NotImplemented_nid_no_patch(__func__);
    return 0;
}

int APS5_VABI sceNgs2ReportRegisterHandler(uint32_t report_type, void* handler, uintptr_t user_data, Ngs2Handle* out_handle) {
    if (handler == nullptr) throw std::invalid_argument("sceNgs2ReportRegisterHandler: null handler");
    if (out_handle == nullptr) throw std::invalid_argument("sceNgs2ReportRegisterHandler: null output handle");
    auto registration = std::make_unique<ReportRegistration>(ReportRegistration{report_type, handler, user_data});
    const Ngs2Handle handle = reinterpret_cast<Ngs2Handle>(registration.get());
    {
        std::lock_guard lock(g_reportMutex);
        g_reportHandlers.emplace(handle, std::move(registration));
    }
    *out_handle = handle;
    return 0;
}

int APS5_VABI sceNgs2ReportUnregisterHandler(Ngs2Handle report_handle) {
    std::lock_guard lock(g_reportMutex);
    if (report_handle == 0 || g_reportHandlers.erase(report_handle) == 0)
        throw std::invalid_argument("sceNgs2ReportUnregisterHandler: invalid report handle");
    return 0;
}

}

#pragma GCC visibility pop
