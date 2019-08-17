/* This file is part of scrab which is
 * licensed under the GPL v2.0
 * github.com/univrsal/scrab
 */
#include <obs-module.h>
#include <util/config-file.h>
#include <obs-frontend-api.h>
#include <QDate>
#include <QDir>

#include "screenshot/screenshotgrabber.hpp"

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("scrab", "en-US")

#define S_HOTKEY_CAPTURE		"scrab.hotkey.capture"

#define T_(t)					obs_module_text(t)
#define T_HOTKEY_CAPTURE_DESC	T_("scrab.hotkey.capture.description")

#define SRC_NAME				"scrab-cap"

static obs_hotkey_id capture_key;
static const char* picuture_folder = nullptr;

/* Gets the scrab source or creates it
 * and sets the file path to the provided one*/
obs_source_t* get_cap_source(const char* path)
{
    obs_source_t* src = obs_get_source_by_name(SRC_NAME);
    obs_data_t* settings;
    if (src) {
        settings = obs_source_get_settings(src);
    } else {
        settings = obs_data_create();
        src = obs_source_create("image_source", SRC_NAME, settings, nullptr);
    }
    obs_data_set_string(settings, "file", path);
    obs_source_update(src, settings);
    obs_data_release(settings);
    return src;
}

void setup_config()
{
    config_t* cfg = obs_frontend_get_global_config();
    config_set_default_string(cfg, "scrab", "image_folder",
                              qPrintable(QDir::homePath()));
    picuture_folder = config_get_string(cfg, "scrab", "image_folder");
}

/* Saves a pixmap into the plugin folder with timestamp
 * and returns the path in path_out */
bool save_pixmap(QPixmap* pixmap, QString& path_out)
{
    bool result = true;
    QString file_name = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss.png");
    path_out = QDir::toNativeSeparators(QDir(picuture_folder).absoluteFilePath(file_name));

    blog(LOG_DEBUG, "[scrab] Filename: %s", qPrintable(file_name));
    blog(LOG_DEBUG, "[scrab] File path: %s", qPrintable(path_out));

    if (!pixmap->save(path_out, "PNG")) {
        blog(LOG_ERROR, "[scrab] Couldn't save screenshot to %s", qPrintable(path_out));
        result = false;
    }
    return result;
}

void screenshot_callback(bool result, QPixmap* arg)
{
    QString path;
    if (result && arg && save_pixmap(arg, path)) {
        /* Set image source */
        auto* scene_src = obs_frontend_get_current_scene();
        auto* scene = obs_scene_from_source(scene_src);
        auto* src = get_cap_source(qPrintable(path));

        if (scene && src) {
            if (!obs_scene_find_source(scene, SRC_NAME)) {
                /* The current scene doesn't have the source yet */
                obs_scene_add(scene, src);
            }
        }

        if (scene_src)
            obs_source_release(scene_src);
        if (src)
            obs_source_release(src);
    }
}

void hot_key_callback(void* data, obs_hotkey_id id, obs_hotkey_t* key,
                      bool pressed)
{
    if (id != capture_key || !pressed)
        return;
    /* Capture screenshot, screenshotgrabber is a QObject and is automatically
     * deleted */
    ScreenshotGrabber* screen_grabber = new ScreenshotGrabber(&screenshot_callback);
    screen_grabber->showGrabber();
}

bool obs_module_load()
{
    setup_config();
    capture_key = obs_hotkey_register_frontend(S_HOTKEY_CAPTURE, T_HOTKEY_CAPTURE_DESC,
                                 hot_key_callback, nullptr);
    return true;
}
