/*!
 * \file
 * \brief GUI формы настроек приложения
 */

#ifndef _ekstertera_forms_form_settings_ui_h_
#define _ekstertera_forms_form_settings_ui_h_

#include "sysheaders.h"

/*!
 * \brief Класс для генерации GUI части формы настроек приложения
 */
class FormSettingsUI : public QDialog {
    Q_OBJECT

public:
    /*!
     * \brief Конструктор
     * \param parent Родительский виджет
     */
    FormSettingsUI(QWidget* parent);
    ~FormSettingsUI();

protected:
    /*!
     * \brief Функция отработки закрытия формы
     */
    void virtual closeEvent(QCloseEvent* event);

    /*!
     * \brief Функция отработки смены состояния формы
     */
    void virtual changeEvent(QEvent* event);

    /*!
     * \brief Функция перевода надписей на элементах управления
     */
    void retranslateUi();

    /*!
     * \brief Функция сохранения расположения и размеров элементов управления формы (при выходе)
     */
    void save();

    /*!
     * \brief Функция восстановления расположения и размеров элементов управления формы (при старте)
     */
    void restore();

protected:
    QHBoxLayout* m_layout;          /*!< \brief Общий layout              */
    QFormLayout* m_layout_settings; /*!< \brief Layout для формы настроек */
    QLabel* m_label_language;       /*!< \brief Язык                      */
    QLabel* m_label_token;          /*!< \brief OAuth токен               */

    QComboBox* m_combo_language; /*!< \brief Язык                      */
    QHBoxLayout* m_layout_token; /*!< \brief Layout для поля токена    */
    QLineEdit* m_text_token;     /*!< \brief Поле OAuth токена         */
    QPushButton* m_button_token; /*!< \brief Получение OAuth токена    */
    QCheckBox* m_close_to_tray;  /*!< \brief Сворачивание в трей       */

    QVBoxLayout* m_layout_button; /*!< \brief Layout для кнопок         */
    QPushButton* m_button_ok;     /*!< \brief Кнопка OK                 */
    QPushButton* m_button_cancel; /*!< \brief Кнопка Отмена             */
    QSpacerItem* m_spacer_button; /*!< \brief Выравнивание кнопок вверх */
};

#endif // _ekstertera_forms_form_settings_ui_h_
