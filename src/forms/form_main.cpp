#include "form_main.h"
//----------------------------------------------------------------------------------------------
#include "utils/settings.h"
//----------------------------------------------------------------------------------------------
#include "form_settings.h"
//----------------------------------------------------------------------------------------------

FormMain::FormMain()
    : FormMainUI() {
    // меню "Файл"
    connect(m_menu_file_exit, &QAction::triggered, this, &FormMain::menu_file_exit_triggered);

    // меню "Параметры"
    connect(m_menu_edit_settings, &QAction::triggered, this, &FormMain::menu_edit_settings_triggered);

    // меню "Вид"
    connect(m_menu_view_refresh, &QAction::triggered, this, &FormMain::menu_view_refresh_triggered);
    connect(m_menu_view_zoom_in, &QAction::triggered, this, &FormMain::menu_view_zoom_in_triggered);
    connect(m_menu_view_zoom_out, &QAction::triggered, this, &FormMain::menu_view_zoom_out_triggered);
    connect(m_menu_view_preview, &QAction::changed, this, &FormMain::menu_view_preview_changed);

    // меню "?"
    connect(m_menu_about, &QAction::triggered, this, &FormMain::menu_help_about_triggered);
    connect(m_menu_about_qt, &QAction::triggered, this, &FormMain::menu_help_about_qt_triggered);

    // иконка трея
    connect(m_tray_icon, &WidgetTrayIcon::activated, this, &FormMain::tray_icon_activated);
    connect(m_tray_menu_show, &QAction::triggered, this, &FormMain::tray_menu_show_triggered);

    // тулбар
    connect(m_action_upload, &QAction::triggered, this, &FormMain::action_upload_triggered);
    connect(m_action_download, &QAction::triggered, this, &FormMain::action_download_triggered);

    // виджет пути
    connect(m_widget_path, &WidgetDiskPath::onPathChangeRequest, this, &FormMain::widget_path_on_path_change_request);

    // виджет диска
    connect(m_widget_disk, &WidgetDisk::onPathChanged, this, &FormMain::widget_disk_on_path_changed);
    connect(m_widget_disk, &WidgetDisk::onChangePossibleActions, this, &FormMain::widget_disk_on_change_possible_actions);
    connect(m_widget_disk, &WidgetDisk::onSelectionChanged, this, &FormMain::widget_disk_on_selection_changed);

    // обновление статуса
    updateInfoStatus();

    // получение корня диска
    m_widget_disk->changePath("disk:/");
}
//----------------------------------------------------------------------------------------------

FormMain::~FormMain() {
}
//----------------------------------------------------------------------------------------------

void FormMain::changeEvent(QEvent* event) {
    if(event->type() == QEvent::LanguageChange) {
        updateInfoStatus();
        m_widget_disk->retranslateUi();
    }

    FormMainUI::changeEvent(event);
}
//----------------------------------------------------------------------------------------------

void FormMain::menu_file_exit_triggered() {
    m_tray_icon->setVisible(false);
    close();
}
//----------------------------------------------------------------------------------------------

void FormMain::menu_edit_settings_triggered() {
    QString token = EteraSettings::instance()->token();

    FormSettings form(this);
    if(form.exec() == QDialog::Accepted) {
        if(token != EteraSettings::instance()->token()) {
            updateInfoStatus();
            m_widget_disk->changePath("disk:/");
        }

        m_tray_icon->setVisible(EteraSettings::instance()->closeToTray());
    }
}
//----------------------------------------------------------------------------------------------

void FormMain::menu_view_refresh_triggered() {
    m_widget_disk->changePath(m_widget_disk->path());
}
//----------------------------------------------------------------------------------------------

void FormMain::menu_view_zoom_in_triggered() {
    bool result = m_widget_disk->zoomIn();

    if(result == false)
        m_menu_view_zoom_in->setEnabled(false);

    m_menu_view_zoom_out->setEnabled(true);
}
//----------------------------------------------------------------------------------------------

void FormMain::menu_view_zoom_out_triggered() {
    bool result = m_widget_disk->zoomOut();

    if(result == false)
        m_menu_view_zoom_out->setEnabled(false);

    m_menu_view_zoom_in->setEnabled(true);
}
//----------------------------------------------------------------------------------------------

void FormMain::menu_view_preview_changed() {
    m_widget_disk->setPreviewMode(m_menu_view_preview->isChecked());
}
//----------------------------------------------------------------------------------------------

void FormMain::menu_help_about_triggered() {
    QString text;
    text += QString("<b>%1 v%2</b><br><br>").arg(ETERA_APP_NAME).arg(ETERA_VERSION);
    text += QString("<a href=\"%1\">%1</a><br>").arg("https://github.com/abbat/ekstertera");

    QMessageBox::about(this, tr("О программе"), text);
}
//----------------------------------------------------------------------------------------------

void FormMain::menu_help_about_qt_triggered() {
    QMessageBox::aboutQt(this, tr("О Qt"));
}
//----------------------------------------------------------------------------------------------

void FormMain::tray_icon_activated(QSystemTrayIcon::ActivationReason reason) {
    if(reason == QSystemTrayIcon::Trigger)
        setVisible(!isVisible());
}
//----------------------------------------------------------------------------------------------

void FormMain::tray_menu_show_triggered() {
    setVisible(true);
}
//----------------------------------------------------------------------------------------------

void FormMain::task_on_info_error(EteraAPI* api) {
    m_label_used->setText("");
    m_label_total->setText("");

    m_label_used->setVisible(false);
    m_label_total->setVisible(false);

    api->deleteLater();
}
//----------------------------------------------------------------------------------------------

void FormMain::task_on_info_success(EteraAPI* api, const EteraInfo& info) {
    m_label_used->setText(EteraAPI::humanBytes(info.used()));
    m_label_total->setText(EteraAPI::humanBytes(info.total()));

    m_label_used->setVisible(true);
    m_label_total->setVisible(true);

    api->deleteLater();
}
//----------------------------------------------------------------------------------------------

void FormMain::updateInfoStatus() {
    QString token = EteraSettings::instance()->token();

    if(token.isEmpty() == true)
        return;

    EteraAPI* api = new EteraAPI(this);

    api->setToken(token);

    ETERA_API_TASK_INFO(api, task_on_info_success, task_on_info_error);

    api->info();
}
//----------------------------------------------------------------------------------------------

void FormMain::widget_path_on_path_change_request(const QString& path) {
    m_widget_disk->changePath(path);
}
//----------------------------------------------------------------------------------------------

void FormMain::widget_disk_on_path_changed(const QString& path) {
    m_widget_path->changePath(path);
}
//----------------------------------------------------------------------------------------------

void FormMain::widget_disk_on_change_possible_actions(bool download) {
    m_action_download->setEnabled(download);
}
//----------------------------------------------------------------------------------------------

void FormMain::widget_disk_on_selection_changed(int files, int dirs, quint64 size) {
    if(files + dirs == 0) {
        m_label_selected->setVisible(false);
        return;
    }

    QString info;
    if(files == 0)
        info = tr("директорий: %1").arg(dirs);
    else if(dirs == 0)
        info = tr("файлов: %1 (%2)").arg(files).arg(EteraAPI::humanBytes(size));
    else
        info = tr("директорий: %1, файлов: %2 (%3)").arg(dirs).arg(files).arg(EteraAPI::humanBytes(size));

    m_label_selected->setVisible(true);
    m_label_selected->setText(info);
}
//----------------------------------------------------------------------------------------------

void FormMain::action_upload_triggered() {
    QStringList list = QFileDialog::getOpenFileNames(this, tr("Загрузка файлов на Яндекс.Диск"));
    m_widget_disk->putLocalObjects(list);
}
//----------------------------------------------------------------------------------------------

void FormMain::action_download_triggered() {
    QString path = QFileDialog::getExistingDirectory(this, tr("Загрузка файлов с Яндекс.Диска"));
    if(path == "")
        return;

    m_widget_disk->getRemoteObjects(path);
}
//----------------------------------------------------------------------------------------------
