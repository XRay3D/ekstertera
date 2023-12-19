#include "settings.h"
//----------------------------------------------------------------------------------------------
static EteraSettings* g_settings = NULL;
//----------------------------------------------------------------------------------------------

void EteraSettings::init() {
    g_settings = new EteraSettings();
}
//----------------------------------------------------------------------------------------------

void EteraSettings::cleanup() {
    delete g_settings;
}
//----------------------------------------------------------------------------------------------

EteraSettings* EteraSettings::instance() {
    return g_settings;
}
//----------------------------------------------------------------------------------------------

EteraSettings::EteraSettings() {
    reload();
}
//----------------------------------------------------------------------------------------------

EteraSettings::~EteraSettings() {
}
//----------------------------------------------------------------------------------------------

void EteraSettings::reload() {
    QSettings settings;

    m_language = settings.value("app/language", "").toString();
    m_token = settings.value("api/token", "").toString();
    m_close_to_tray = settings.value("app/tray", 1).toBool();
}
//----------------------------------------------------------------------------------------------
