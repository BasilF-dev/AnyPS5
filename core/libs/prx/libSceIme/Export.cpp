#include <cstdint>
#include <cstddef>
#include <cstring>
#include <mutex>
#include <set>
#include <stdexcept>
#include <string>
#include "SceTypes.hpp"
#include "prx/libc/include/General.hpp"

namespace {

constexpr int ErrorBusy = static_cast<int>(0x80bc0001u);
constexpr int ErrorNotOpened = static_cast<int>(0x80bc0002u);
constexpr int ErrorConnectionFailed = static_cast<int>(0x80bc0004u);
constexpr int ErrorInvalidText = static_cast<int>(0x80bc0006u);
constexpr int ErrorInvalidUserId = static_cast<int>(0x80bc0010u);
constexpr int ErrorInvalidType = static_cast<int>(0x80bc0011u);
constexpr int ErrorInvalidOption = static_cast<int>(0x80bc0015u);
constexpr int ErrorInvalidMaxTextLength = static_cast<int>(0x80bc0016u);
constexpr int ErrorInvalidInputTextBuffer = static_cast<int>(0x80bc0017u);
constexpr int ErrorInvalidPosX = static_cast<int>(0x80bc0018u);
constexpr int ErrorInvalidPosY = static_cast<int>(0x80bc0019u);
constexpr int ErrorInvalidHorizontalAlignment = static_cast<int>(0x80bc001au);
constexpr int ErrorInvalidVerticalAlignment = static_cast<int>(0x80bc001bu);
constexpr int ErrorInvalidWork = static_cast<int>(0x80bc0020u);
constexpr int ErrorInvalidHandler = static_cast<int>(0x80bc0022u);
constexpr int ErrorNoResourceId = static_cast<int>(0x80bc0023u);
constexpr int ErrorInvalidMode = static_cast<int>(0x80bc0024u);
constexpr int ErrorInvalidParam = static_cast<int>(0x80bc0030u);
constexpr int ErrorInvalidAddress = static_cast<int>(0x80bc0031u);
constexpr int ErrorInvalidReserved = static_cast<int>(0x80bc0032u);
constexpr uint32_t ValidKeyboardModes = 0x0000007f;
constexpr uint32_t TypeNumber = 4;
constexpr uint32_t ValidOptions = 0x00007bff;
constexpr uint32_t OptionMultiline = 0x00000001;
constexpr uint32_t OptionPassword = 0x00000004;
constexpr uint32_t OptionUseOver2K = 0x00004000;
constexpr uint32_t MaxTextLength = 2048;
constexpr int32_t UserIdInvalid = -1;

struct PanelState {
    bool open = false;
    char16_t* inputTextBuffer = nullptr;
    uint32_t maxTextLength = 0;
    uint32_t textLength = 0;
    uint32_t option = 0;
    PositionAndForm form{};
    EventHandler handler = nullptr;
    void* argument = nullptr;
    Caret caret{};
    TextGeometry geometry{};
    TextAreaMode geometryMode = TextAreaMode::Disable;
};

uint32_t BoundedTextLength(const char16_t* text, uint32_t limit) {
    if (!text) return 0;
    uint32_t length = 0;
    while (length < limit && text[length] != u'\0') ++length;
    return length;
}

bool IsValidText(const char16_t* text, uint32_t length, bool multiline) {
    if (!text) return false;
    for (uint32_t index = 0; index < length; ++index) {
        const char16_t value = text[index];
        if (value == u'\0') break;
        if (!multiline && (value == u'\n' || value == u'\r')) return false;
        if ((static_cast<uint16_t>(value) & 0xf800u) == 0xd800u) return false;
    }
    return true;
}

std::mutex g_panelMutex;
PanelState g_panel;
std::mutex g_keyboardMutex;
std::set<int32_t> g_openKeyboards;

}

extern "C" {

int APS5_VABI sceImeClose_nid_postfix(void) {
    std::lock_guard lock(g_panelMutex);
    if (!g_panel.open) return ErrorNotOpened;
    g_panel = {};
    return 0;
}

int APS5_VABI sceImeGetPanelPositionAndForm(PositionAndForm* form) {
    std::lock_guard lock(g_panelMutex);
    if (!g_panel.open) return ErrorNotOpened;
    if (!form) return ErrorInvalidAddress;
    *form = g_panel.form;
    return 0;
}

int APS5_VABI sceImeGetPanelSize(const Param* param, uint32_t* width, uint32_t* height) {
 if (!param || !width || !height) return ErrorInvalidAddress;
 if (param->type > TypeNumber) return ErrorInvalidType;
 if ((param->option & ~ValidOptions) != 0) return ErrorInvalidOption;
 const uint32_t scale = (param->option & OptionUseOver2K) != 0 ? 2 : 1;
 *width = (param->type == TypeNumber ? 370u : 793u) * scale;
 *height = (param->type == TypeNumber ? 402u : 408u) * scale;
 return 0;
}

int APS5_VABI sceImeKeyboardClose(int32_t user_id) {
 std::lock_guard lock(g_keyboardMutex);
 if (g_openKeyboards.erase(user_id) == 0) throw std::logic_error("sceImeKeyboardClose: keyboard not open for user " + std::to_string(user_id));
 return 0;
}

int APS5_VABI sceImeKeyboardGetInfo(uint32_t resource_id, KeyboardInfo* info) {
 (void)resource_id;
 if (!info) return ErrorInvalidAddress;
 std::lock_guard lock(g_keyboardMutex);
 return g_openKeyboards.empty() ? ErrorNotOpened : ErrorNoResourceId;
}

int APS5_VABI sceImeKeyboardGetResourceId(int32_t user_id, KeyboardResourceIdArray* resource_ids) {
 if (!resource_ids) return ErrorInvalidAddress;
 if (user_id == UserIdInvalid) return ErrorInvalidUserId;
 *resource_ids = {};
 resource_ids->user_id = user_id;
 std::lock_guard lock(g_keyboardMutex);
 return g_openKeyboards.contains(user_id) ? ErrorConnectionFailed : ErrorNotOpened;
}

int APS5_VABI sceImeKeyboardOpen(int32_t user_id, const KeyboardParam* param) {
 if (!param) APS5_INVALID_ARG_EX;
 std::lock_guard lock(g_keyboardMutex);
 if (!g_openKeyboards.insert(user_id).second) throw std::logic_error("sceImeKeyboardOpen: keyboard already open for user " + std::to_string(user_id));
 return 0;
}

int APS5_VABI sceImeKeyboardSetMode(int32_t user_id, uint32_t mode) {
 if (user_id == UserIdInvalid) return ErrorInvalidUserId;
 std::lock_guard lock(g_keyboardMutex);
 if (!g_openKeyboards.contains(user_id)) return ErrorNotOpened;
 return (mode & ~ValidKeyboardModes) == 0 ? 0 : ErrorInvalidMode;
}

int APS5_VABI sceImeOpen_nid_postfix(const Param* param, const ExtendedParam* extended) {
    (void)extended;
    std::lock_guard lock(g_panelMutex);
    if (g_panel.open) return ErrorBusy;
    if (!param) return ErrorInvalidAddress;
    if (param->user_id == UserIdInvalid) return ErrorInvalidUserId;
    if (param->type > TypeNumber) return ErrorInvalidType;
    if ((param->option & ~ValidOptions) != 0) return ErrorInvalidOption;
    const bool multiline = (param->option & OptionMultiline) != 0;
    const bool password = (param->option & OptionPassword) != 0;
    if ((multiline && password) || (multiline && param->type > 1) ||
        (password && param->type != 1 && param->type != TypeNumber)) return ErrorInvalidParam;
    if (param->max_text_length == 0 || param->max_text_length > MaxTextLength) return ErrorInvalidMaxTextLength;
    if (!param->input_text_buffer) return ErrorInvalidInputTextBuffer;
    const float widthLimit = (param->option & OptionUseOver2K) != 0 ? 3840.0f : 1920.0f;
    const float heightLimit = (param->option & OptionUseOver2K) != 0 ? 2160.0f : 1080.0f;
    if (param->posx < 0.0f || param->posx >= widthLimit) return ErrorInvalidPosX;
    if (param->posy < 0.0f || param->posy >= heightLimit) return ErrorInvalidPosY;
    if (param->horizontal_alignment > 2) return ErrorInvalidHorizontalAlignment;
    if (param->vertical_alignment > 2) return ErrorInvalidVerticalAlignment;
    if (!param->work || (reinterpret_cast<uintptr_t>(param->work) & 3u) != 0) return ErrorInvalidWork;
    if (!param->handler) return ErrorInvalidHandler;
    for (const int8_t value : param->reserved) if (value != 0) return ErrorInvalidReserved;
    const uint32_t textLength = BoundedTextLength(param->input_text_buffer, param->max_text_length);
    if (!IsValidText(param->input_text_buffer, textLength, multiline)) return ErrorInvalidText;

    const uint32_t scale = (param->option & OptionUseOver2K) != 0 ? 2 : 1;
    g_panel.open = true;
    g_panel.inputTextBuffer = param->input_text_buffer;
    g_panel.maxTextLength = param->max_text_length;
    g_panel.textLength = textLength;
    g_panel.option = param->option;
    g_panel.handler = param->handler;
    g_panel.argument = param->arg;
    g_panel.form.type = param->type;
    g_panel.form.posx = param->posx;
    g_panel.form.posy = param->posy;
    g_panel.form.horizontal_alignment = param->horizontal_alignment;
    g_panel.form.vertical_alignment = param->vertical_alignment;
    g_panel.form.width = (param->type == TypeNumber ? 370u : 793u) * scale;
    g_panel.form.height = (param->type == TypeNumber ? 402u : 408u) * scale;
    return 0;
}

void APS5_VABI sceImeParamInit(Param* param) {
 if (!param) return;
 std::memset(param, 0, sizeof(*param));
 param->user_id = -1;
}

int APS5_VABI sceImeSetCaret(const Caret* caret) {
    std::lock_guard lock(g_panelMutex);
    if (!g_panel.open) return ErrorNotOpened;
    if (!caret) return ErrorInvalidAddress;
    if (caret->index > g_panel.textLength) return ErrorInvalidParam;
    g_panel.caret = *caret;
    return 0;
}

int APS5_VABI sceImeSetText(const char16_t* text, uint32_t length) {
    std::lock_guard lock(g_panelMutex);
    if (!g_panel.open) return ErrorNotOpened;
    if (!text) return ErrorInvalidAddress;
    if (length > g_panel.maxTextLength ||
        !IsValidText(text, length, (g_panel.option & OptionMultiline) != 0)) return ErrorInvalidText;
    if (length != 0) std::memmove(g_panel.inputTextBuffer, text, static_cast<size_t>(length) * sizeof(char16_t));
    if (length < g_panel.maxTextLength) g_panel.inputTextBuffer[length] = u'\0';
    g_panel.textLength = length;
    if (g_panel.caret.index > length) g_panel.caret.index = length;
    return 0;
}

int APS5_VABI sceImeSetTextGeometry(TextAreaMode mode, const TextGeometry* geometry) {
    std::lock_guard lock(g_panelMutex);
    if (!g_panel.open) return ErrorNotOpened;
    if (!geometry) return ErrorInvalidAddress;
    if (mode != TextAreaMode::Select && mode != TextAreaMode::Preedit) return ErrorInvalidParam;
    const float widthLimit = (g_panel.option & OptionUseOver2K) != 0 ? 3840.0f : 1920.0f;
    const float heightLimit = (g_panel.option & OptionUseOver2K) != 0 ? 2160.0f : 1080.0f;
    if (geometry->x < 0.0f || geometry->x >= widthLimit ||
        geometry->y < 0.0f || geometry->y >= heightLimit) return ErrorInvalidParam;
    g_panel.geometryMode = mode;
    g_panel.geometry = *geometry;
    return 0;
}

int APS5_VABI sceImeUpdate(EventHandler handler) {
    if (!handler) APS5_INVALID_ARG_EX;
    {
        std::lock_guard lock(g_panelMutex);
        if (g_panel.open && handler == g_panel.handler) return 0;
    }
    std::lock_guard keyboardLock(g_keyboardMutex);
    return g_openKeyboards.empty() ? ErrorNotOpened : 0;
}

}
