#include "settings.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <map>
#include <cstdio>

Settings g_settings;

static std::map<std::string, std::string> parse_ini(const char* path) {
    std::map<std::string, std::string> kv;
    std::ifstream f(path);
    if (!f.is_open()) return kv;

    std::string line;
    while (std::getline(f, line)) {
        auto pos = line.find(';');
        if (pos != std::string::npos) line.erase(pos);
        pos = line.find('#');
        if (pos != std::string::npos) line.erase(pos);

        auto eq = line.find('=');
        if (eq == std::string::npos) continue;

        std::string key = line.substr(0, eq);
        std::string val = line.substr(eq + 1);

        auto trim = [](std::string& s) {
            s.erase(0, s.find_first_not_of(" \t\r\n"));
            s.erase(s.find_last_not_of(" \t\r\n") + 1);
        };
        trim(key);
        trim(val);

        kv[key] = val;
    }
    return kv;
}

static int clamp_int(int val, int lo, int hi) {
    if (val < lo) return lo;
    if (val > hi) return hi;
    return val;
}

void settings_load() {
    auto kv = parse_ini(Settings::INI_PATH);
    if (kv.empty()) {
        settings_save();
        return;
    }

    auto read_bool = [&](const char* key, bool& out) {
        auto it = kv.find(key);
        if (it != kv.end()) out = (it->second != "0");
    };
    auto read_int = [&](const char* key, int& out, int lo, int hi) {
        auto it = kv.find(key);
        if (it != kv.end()) {
            try { out = clamp_int(std::stoi(it->second), lo, hi); }
            catch (...) {}
        }
    };

    read_bool("enabled",                g_settings.enabled);
    read_bool("cinematic_skip_enabled", g_settings.cinematic_skip_enabled);
    read_int ("cinematic_timeout_ms",   g_settings.cinematic_timeout_ms, 0, 60000);
    read_bool("dialogue_skip_enabled",  g_settings.dialogue_skip_enabled);
    read_bool("override_delete_confirm",  g_settings.override_delete_confirm);
    read_bool("override_dragdrop",        g_settings.override_dragdrop);
    read_bool("override_salvage_confirm", g_settings.override_salvage_confirm);
    read_bool("bank_fix_enabled",         g_settings.bank_fix_enabled);
    read_bool("hide_clones_enabled",      g_settings.hide_clones_enabled);
}

void settings_save() {
    CreateDirectoryA("addons", nullptr);
    CreateDirectoryA("addons\\arcdps", nullptr);

    const char* tmp_path = "addons\\arcdps\\arcdps_qol_tweaks.ini.tmp";

    std::ofstream f(tmp_path);
    if (!f.is_open()) return;

    f << "[general]\n";
    f << "enabled=" << (g_settings.enabled ? 1 : 0) << "\n";
    f << "cinematic_skip_enabled=" << (g_settings.cinematic_skip_enabled ? 1 : 0) << "\n";
    f << "cinematic_timeout_ms=" << g_settings.cinematic_timeout_ms << "\n";
    f << "dialogue_skip_enabled=" << (g_settings.dialogue_skip_enabled ? 1 : 0) << "\n";
    f << "override_delete_confirm=" << (g_settings.override_delete_confirm ? 1 : 0) << "\n";
    f << "override_dragdrop=" << (g_settings.override_dragdrop ? 1 : 0) << "\n";
    f << "override_salvage_confirm=" << (g_settings.override_salvage_confirm ? 1 : 0) << "\n";
    f << "bank_fix_enabled=" << (g_settings.bank_fix_enabled ? 1 : 0) << "\n";
    f << "hide_clones_enabled=" << (g_settings.hide_clones_enabled ? 1 : 0) << "\n";
    f.close();

    // Atomic replace; MoveFileEx is NTFS-safe, fallback for Wine
    if (!MoveFileExA(tmp_path, Settings::INI_PATH, MOVEFILE_REPLACE_EXISTING)) {
        DeleteFileA(Settings::INI_PATH);
        MoveFileA(tmp_path, Settings::INI_PATH);
    }
}