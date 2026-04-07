#pragma once

#include <cstdint>
#include <string>
#include <windows.h>

struct Settings {
    bool enabled = true;

    bool cinematic_skip_enabled = true;
    int  cinematic_timeout_ms = 150;

    bool dialogue_skip_enabled = false;

    bool override_delete_confirm = false;
    bool override_dragdrop = false;
    bool override_salvage_confirm = false;

    bool bank_fix_enabled = false;

    bool hide_clones_enabled = false;

    static constexpr const char* INI_PATH = "addons\\arcdps\\arcdps_qol_tweaks.ini";
};

extern Settings g_settings;

void settings_load();
void settings_save();

