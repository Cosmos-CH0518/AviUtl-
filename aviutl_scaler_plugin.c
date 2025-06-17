#include <windows.h>
#include <commctrl.h>
#include <math.h>
#include "aviutl_scaler_plugin.h"
#include "resource.h"

#define CONFIG_REG_KEY L"Software\\AviUtlScalerPlugin"


BOOL CALLBACK ConfigDialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    static PluginConfig* config;

    switch(msg) {
        case WM_INITDIALOG:
            config = (PluginConfig*)lParam;
            SetDlgItemInt(hwnd, IDC_WIDTH, config->target_width, FALSE);
            SetDlgItemInt(hwnd, IDC_HEIGHT, config->target_height, FALSE);
            CheckDlgButton(hwnd, IDC_KEEP_ASPECT, config->keep_aspect ? BST_CHECKED : BST_UNCHECKED);
            return TRUE;

        case WM_COMMAND:
            switch(LOWORD(wParam)) {
                case IDOK:
                    config->target_width = GetDlgItemInt(hwnd, IDC_WIDTH, NULL, FALSE);
                    config->target_height = GetDlgItemInt(hwnd, IDC_HEIGHT, NULL, FALSE);
                    config->keep_aspect = (IsDlgButtonChecked(hwnd, IDC_KEEP_ASPECT) == BST_CHECKED);
                    EndDialog(hwnd, IDOK);
                    return TRUE;
                case IDCANCEL:
                    EndDialog(hwnd, IDCANCEL);
                    return TRUE;
            }
            break;
    }
    return FALSE;
}


void LoadConfig(PluginConfig* config) {
    HKEY hKey;
    DWORD dwType, dwSize;
    if(RegOpenKeyEx(HKEY_CURRENT_USER, CONFIG_REG_KEY, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        dwSize = sizeof(DWORD);
        RegQueryValueEx(hKey, L"TargetWidth", NULL, &dwType, (LPBYTE)&config->target_width, &dwSize);
        RegQueryValueEx(hKey, L"TargetHeight", NULL, &dwType, (LPBYTE)&config->target_height, &dwSize);
        RegQueryValueEx(hKey, L"KeepAspect", NULL, &dwType, (LPBYTE)&config->keep_aspect, &dwSize);
        RegCloseKey(hKey);
    }
}


void SaveConfig(const PluginConfig* config) {
    HKEY hKey;
    if(RegCreateKeyEx(HKEY_CURRENT_USER, CONFIG_REG_KEY, 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS) {
        RegSetValueEx(hKey, L"TargetWidth", 0, REG_DWORD, (const BYTE*)&config->target_width, sizeof(DWORD));
        RegSetValueEx(hKey, L"TargetHeight", 0, REG_DWORD, (const BYTE*)&config->target_height, sizeof(DWORD));
        RegSetValueEx(hKey, L"KeepAspect", 0, REG_DWORD, (const BYTE*)&config->keep_aspect, sizeof(DWORD));
        RegCloseKey(hKey);
    }
}


BOOL func_proc(void *editp, OutputInfo *oip) {
    static PluginConfig config = {1280, 720, TRUE};
    LoadConfig(&config);

    int original_w = 0, original_h = 0;
    GetProjectResolution(editp, &original_w, &original_h);

    if(original_w == 0 || original_h == 0) {
        original_w = 1280;
        original_h = 720;
    }

    if(config.keep_aspect) {
        double aspect = (double)original_w / original_h;
        if(config.target_width == 0 && config.target_height > 0) {
            config.target_width = (int)(config.target_height * aspect);
        } else if(config.target_height == 0 && config.target_width > 0) {
            config.target_height = (int)(config.target_width / aspect);
        }
    }

    double scale_x = (double)config.target_width / original_w;
    double scale_y = (double)config.target_height / original_h;

    int object_count = GetObjectCount(editp);
    for(int i = 0; i < object_count; i++) {
        Object* obj = GetObject(editp, i);
        if(!obj) continue;

        obj->scale_x = (int)lround(obj->scale_x * scale_x);
        obj->scale_y = (int)lround(obj->scale_y * scale_y);

        int offset_x = (config.target_width - original_w) / 2;
        int offset_y = (config.target_height - original_h) / 2;
        obj->x = (int)lround(obj->x * scale_x) + offset_x;
        obj->y = (int)lround(obj->y * scale_y) + offset_y;
    }

    oip->width = config.target_width;
    oip->height = config.target_height;

    return TRUE;
}


BOOL config_proc(void *editp, void *data) {
    PluginConfig config;
    LoadConfig(&config);

    if(DialogBoxParam(GetModuleHandle(NULL),
                     MAKEINTRESOURCE(IDD_CONFIG),
                     GetActiveWindow(),
                     ConfigDialogProc,
                     (LPARAM)&config) == IDOK) {
        SaveConfig(&config);
        return TRUE;
    }
    return FALSE;
}


EXPORT OutputPlugin *GetOutputPluginTable(void) {
    static OutputPlugin plugin = {
        0,
        "出力解像度スケーラー",
        func_proc,
        config_proc,
        NULL,
        NULL,
        NULL,
        NULL,
        0
    };
    return &plugin;
}
