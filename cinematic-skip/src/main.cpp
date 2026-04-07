#include "globals.h"
#include "settings.h"
#include "skip_logic.h"

#include <imgui.h>
#include <windows.h>

typedef struct arcdps_exports {
    uintptr_t    size;
    uint32_t     sig;
    uint32_t     imguivers;
    const char*  out_name;
    const char*  out_build;
    void*        wnd_nofilter;
    void*        combat;
    void*        imgui;
    void*        options_end;
    void*        combat_local;
    void*        wnd_filter;
    void*        options_windows;
} arcdps_exports;

static arcdps_exports arc_exports;
static bool s_show_window = true;

static uintptr_t mod_imgui(uint32_t not_charsel_or_loading) {
    skip_tick();

    if (!s_show_window) return 0;

    if (ImGui::Begin(ADDON_NAME, &s_show_window, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Checkbox("Enabled", &g_settings.enabled);
        ImGui::Separator();

        ImGui::Checkbox("Skip Cinematics", &g_settings.cinematic_skip_enabled);
        ImGui::InputInt("Timeout (ms)##cine", &g_settings.cinematic_timeout_ms, 50, 500);
        if (g_settings.cinematic_timeout_ms < 0) g_settings.cinematic_timeout_ms = 0;

        ImGui::Checkbox("Skip Dialogue", &g_settings.dialogue_skip_enabled);

        ImGui::Separator();
        ImGui::Text("Confirmation Overrides");
        ImGui::Checkbox("Skip Delete Confirm", &g_settings.override_delete_confirm);
        ImGui::Checkbox("Skip Drag-Drop Confirm", &g_settings.override_dragdrop);

        ImGui::Separator();
        ImGui::Text("Visual");
        if (ImGui::Checkbox("Hide Clones & Phantasms", &g_settings.hide_clones_enabled)) {
            settings_save();
        }
        ImGui::TextDisabled("  Applies to newly loaded models. Reload instance for full refresh.");

        ImGui::Separator();
        ImGui::Text("Performance");
        bool bank_prev = g_settings.bank_fix_enabled;
        ImGui::Checkbox("Fix Bank Opening Lag", &g_settings.bank_fix_enabled);
        if (g_settings.bank_fix_enabled != bank_prev) {
            apply_bank_fix(g_settings.bank_fix_enabled);
            settings_save();
        }

        ImGui::Separator();

        bool cin_ok   = is_cinematic_hook_active();
        bool dlg_ok   = is_dialogue_hook_active() && g_dlg_complete_fn != nullptr;
        bool ctx_ok   = g_ccontext_get != nullptr;
        bool tick_ok  = is_game_tick_hook_active();
        bool conf_ok  = is_confirm_hook_active();
        bool drag_ok  = is_dragdrop_hook_active();
        bool bank_ok  = is_bank_fix_active();
        bool model_ok = is_model_load_hook_active();

        if (cin_ok && dlg_ok && ctx_ok && tick_ok && conf_ok && drag_ok && bank_ok && model_ok) {
            ImGui::TextColored(ImVec4(0, 1, 0, 1), "Status: OK");
        } else {
            if (!ctx_ok)   ImGui::TextColored(ImVec4(1, 0.3f, 0.3f, 1), "CContext: scan failed");
            if (!cin_ok)   ImGui::TextColored(ImVec4(1, 0.3f, 0.3f, 1), "Cinematic hook: not found");
            if (!dlg_ok)   ImGui::TextColored(ImVec4(1, 0.3f, 0.3f, 1), "Dialogue hook: not found");
            if (!tick_ok)  ImGui::TextColored(ImVec4(1, 0.3f, 0.3f, 1), "Game tick hook: not found");
            if (!conf_ok)  ImGui::TextColored(ImVec4(1, 0.3f, 0.3f, 1), "Confirm hook: not found");
            if (!drag_ok)  ImGui::TextColored(ImVec4(1, 0.3f, 0.3f, 1), "Dragdrop hook: not found");
            if (!bank_ok)  ImGui::TextColored(ImVec4(1, 0.3f, 0.3f, 1), "Bank fix: scan failed");
            if (!model_ok) ImGui::TextColored(ImVec4(1, 0.3f, 0.3f, 1), "Model load hook: not found");
        }

        if (is_cinematic_active()) {
            ImGui::TextColored(ImVec4(1, 1, 0, 1), "Cinematic playing...");
        }
        if (is_dialogue_active()) {
            ImGui::TextColored(ImVec4(1, 1, 0, 1), "Dialogue playing...");
        }

    }
    ImGui::End();

    return 0;
}

static uintptr_t mod_options_end() {
    ImGui::Checkbox(ADDON_NAME, &s_show_window);
    ImGui::SameLine();
    ImGui::TextDisabled("(Alt+Shift+C)");
    return 0;
}

static uintptr_t mod_wnd(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (g_gw2_hwnd == nullptr && hWnd != nullptr) {
        g_gw2_hwnd = hWnd;
    }

    if (g_ccontext_get != nullptr) {
        __try {
            uintptr_t ctx = g_ccontext_get();
            if (ctx != 0) {
                g_ccontext_cached = ctx;
            }
        } __except (EXCEPTION_EXECUTE_HANDLER) {}
    }

    // Alt+Shift+C toggles window
    if ((uMsg == WM_KEYDOWN || uMsg == WM_SYSKEYDOWN) && wParam == 'C') {
        bool alt_held   = (GetKeyState(VK_MENU) & 0x8000) != 0;
        bool shift_held = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
        if (alt_held && shift_held) {
            s_show_window = !s_show_window;
            return 0;
        }
    }

    return uMsg;
}

static arcdps_exports* mod_init() {
    settings_load();
    skip_init();

    memset(&arc_exports, 0, sizeof(arc_exports));
    arc_exports.size            = sizeof(arcdps_exports);
    arc_exports.sig             = ADDON_SIG;
    arc_exports.imguivers       = IMGUI_VERSION_NUM;
    arc_exports.out_name        = ADDON_NAME;
    arc_exports.out_build       = ADDON_VERSION;
    arc_exports.wnd_nofilter    = reinterpret_cast<void*>(mod_wnd);
    arc_exports.imgui           = reinterpret_cast<void*>(mod_imgui);
    arc_exports.options_end     = reinterpret_cast<void*>(mod_options_end);

    return &arc_exports;
}

static uintptr_t mod_release() {
    settings_save();
    skip_shutdown();
    return 0;
}

extern "C" __declspec(dllexport) void* get_init_addr(
    char* arcversionstr,
    ImGuiContext* imguictx,
    void* id3dptr,
    HANDLE arcdll,
    void* mallocfn,
    void* freefn,
    uint32_t d3dversion)
{
    ImGui::SetCurrentContext(imguictx);
    ImGui::SetAllocatorFunctions(
        reinterpret_cast<void*(*)(size_t, void*)>(mallocfn),
        reinterpret_cast<void(*)(void*, void*)>(freefn));

    return reinterpret_cast<void*>(mod_init);
}

extern "C" __declspec(dllexport) void* get_release_addr() {
    return reinterpret_cast<void*>(mod_release);
}
