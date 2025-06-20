#ifndef AVIUTL_SCALER_PLUGIN_H
#define AVIUTL_SCALER_PLUGIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include <windows.h>


typedef struct {
    int flag;
    int width, height;
    int rate, scale;
    int frame_n;
    int audio_rate;
    int audio_ch;
    int audio_n;
    int format;
    int sample_n;
    void *editp;
} OutputInfo;


typedef struct {
    int flag;
    char *name;
    BOOL (*func_proc)(void *editp, OutputInfo *oip);
    BOOL (*config_proc)(void *editp, void *data);
    void (*save_config_proc)(void *data, int size);
    void (*project_change_proc)(void *editp);
    void (*exit_proc)(void);
    void (*get_version_proc)(int *ver, int *build);
    int reserved;
} OutputPlugin;


typedef struct {
    int target_width;
    int target_height;
    BOOL keep_aspect;
} PluginConfig;

// AviUtl拡張編集オブジェクトの例（実際は拡張編集SDKに合わせて修正）
typedef struct {
    int x, y;
    int scale_x, scale_y;
} Object;


void GetProjectResolution(void *editp, int *width, int *height);
int GetObjectCount(void *editp);
Object* GetObject(void *editp, int index);


#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif
EXPORT OutputPlugin *GetOutputPluginTable(void);

#ifdef __cplusplus
}
#endif

#endif // AVIUTL_SCALER_PLUGIN_H
