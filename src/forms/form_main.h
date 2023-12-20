/*!
 * \file
 * \brief Главная форма приложения
 */

#ifndef _ekstertera_forms_form_main_h_
#define _ekstertera_forms_form_main_h_

#include "form_main_ui.h"

/*!
 * \brief Класс главной формы приложения
 */
class FormMain : public FormMainUI {
    Q_OBJECT

public:
    FormMain();
    ~FormMain();

protected:
    /*!
     * \brief Функция отработки смены состояния формы
     */
    void virtual changeEvent(QEvent* event);

    void dragEnterEvent(QDragEnterEvent* event) override;
    //----------------------------------------------------------------------------------------------

    void dropEvent(QDropEvent* event) override;

private:
    /*!
     * \brief Обновление статуса в статус-баре
     */
    void updateInfoStatus();

private slots:

    //
    // меню
    //

    // Файл
    void menu_file_exit_triggered(); /*!< \brief Файл / Выход */

    // Параметры
    void menu_edit_settings_triggered(); /*!< \brief Параметры / Настройки */

    // Вид
    void menu_view_refresh_triggered();  /*!< \brief Обновить     */
    void menu_view_zoom_in_triggered();  /*!< \brief Увеличить    */
    void menu_view_zoom_out_triggered(); /*!< \brief Уменьшить    */
    void menu_view_preview_changed();    /*!< \brief Предпросмотр */

    // ?
    void menu_help_about_triggered();    /*!< \brief ? / О программе */
    void menu_help_about_qt_triggered(); /*!< \brief ? / О Qt        */

    //
    // обработчики иконки трея
    //

    void tray_icon_activated(QSystemTrayIcon::ActivationReason reason); /*!< \brief Скрыть / Показать */
    void tray_menu_show_triggered();                                    /*!< \brief Показать */

    //
    // тулбар
    //

    void action_upload_triggered();   /*!< \brief Загрузить на Яндекс.Диск */
    void action_download_triggered(); /*!< \brief Загрузить с Яндекс.Диска */

    //
    // обработчики виджета диска
    //

    void widget_disk_on_path_changed(const QString& path);                    /*!< \brief Смена пути в виджете     */
    void widget_disk_on_change_possible_actions(bool download);               /*!< \brief Смена возможных действий */
    void widget_disk_on_selection_changed(int files, int dirs, quint64 size); /*!< \brief Смена выделения          */

    //
    // обработчики виджета путей
    //

    void widget_path_on_path_change_request(const QString& path); /*!< \brief Запрос на смену пути */

    //
    // обработчики получения информации о диске
    //

    void task_on_info_error(EteraAPI* api);                          /*!< \brief Ошибка обновления информации о диске   */
    void task_on_info_success(EteraAPI* api, const EteraInfo& info); /*!< \brief Успешное обновление информации о диске */
};

#endif // _ekstertera_forms_form_main_h_
