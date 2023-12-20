#include "form_settings.h"
//----------------------------------------------------------------------------------------------
#include "utils/settings.h"
#include "utils/translator.h"
//----------------------------------------------------------------------------------------------

FormSettings::FormSettings(QWidget* parent)
    : FormSettingsUI(parent) {
    EteraSettings* settings = EteraSettings::instance();

    m_language = EteraTranslator::instance()->language();

    m_text_token->setText(settings->token());

    int index = m_combo_language->findData(m_language);
    if(index != -1)
        m_combo_language->setCurrentIndex(index);

    m_close_to_tray->setChecked(settings->closeToTray());

    connect(m_button_cancel, &QPushButton::clicked, this, &FormSettings::button_cancel_clicked);
    connect(m_button_ok, &QPushButton::clicked, this, &FormSettings::button_ok_clicked);

    connect(m_combo_language, &QComboBox::currentIndexChanged, this, &FormSettings::combo_language_current_index_changed);
    connect(m_button_token, &QPushButton::clicked, this, &FormSettings::button_token_clicked);
}
//----------------------------------------------------------------------------------------------

FormSettings::~FormSettings() {
}
//----------------------------------------------------------------------------------------------

void FormSettings::changeEvent(QEvent* event) {
    FormSettingsUI::changeEvent(event);
}
//----------------------------------------------------------------------------------------------

void FormSettings::combo_language_current_index_changed(int index) {
    QString language = m_combo_language->itemData(index).toString();

    EteraTranslator::instance()->changeTranslator(language);
}
//----------------------------------------------------------------------------------------------

void FormSettings::button_cancel_clicked() {
    // возврат локализации
    QString language = m_combo_language->itemData(m_combo_language->currentIndex()).toString();
    if(language != m_language)
        EteraTranslator::instance()->changeTranslator(m_language);

    save();
    reject();
}
//----------------------------------------------------------------------------------------------

void FormSettings::button_ok_clicked() {
    QSettings settings;

    settings.setValue("app/language", m_combo_language->itemData(m_combo_language->currentIndex()).toString());
    settings.setValue("api/token", m_text_token->text());
    settings.setValue("app/tray", m_close_to_tray->isChecked() == true ? 1 : 0);

    EteraSettings::instance()->reload();

    save();
    accept();
}
//----------------------------------------------------------------------------------------------

void FormSettings::button_token_clicked() {
    EteraAPI* api = new EteraAPI(this);

    api->getToken();

    bool ok;
    QString auth_code = QInputDialog::getText(this, tr("Получение OAuth токена"), tr("Введите код подтверждения"), QLineEdit::Normal, "", &ok).trimmed();
    if(ok == false || auth_code.isEmpty() == true) {
        delete api;
        QMessageBox::warning(this, tr("Внимание!"), tr("Необходимо ввести код подтверждения!"));
        return;
    }
    
    connect(api, &EteraAPI::onERROR, this, &FormSettings::task_on_token_error);
    connect(api, &EteraAPI::onTOKEN, this, &FormSettings::task_on_token_success);

    api->getToken(auth_code);
}
//----------------------------------------------------------------------------------------------

void FormSettings::task_on_token_error(EteraAPI* api) {
    QMessageBox::critical(this, tr("Ошибка!"), api->lastErrorMessage());
    api->deleteLater();
}
//----------------------------------------------------------------------------------------------

void FormSettings::task_on_token_success(EteraAPI* api, const QString& token) {
    m_text_token->setText(token);
    api->deleteLater();
}
//----------------------------------------------------------------------------------------------
