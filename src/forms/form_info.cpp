#include "form_info.h"
//----------------------------------------------------------------------------------------------

FormInfo::FormInfo(const EteraItem& item, QWidget* parent)
    : FormInfoUI(parent) {
    m_item = item;

    m_text_name->setText(m_item.name());
    m_text_path->setText(m_item.path());

    QString format = "yyyy-MM-dd hh:mm:ss";
    m_text_created->setText(m_item.created().toLocalTime().toString(format));
    m_text_modified->setText(m_item.modified().toLocalTime().toString(format));

    m_text_size->setText(EteraAPI::humanBytes(m_item.size()));
    m_text_md5->setText(m_item.md5());
    m_text_sha256->setText(m_item.sha256());
    m_text_media->setText(m_item.mediaString());
    m_text_mime->setText(m_item.mimeType());
    m_text_public_url->setText(m_item.publicURL());

    retranslateUi();
}
//----------------------------------------------------------------------------------------------

FormInfo::~FormInfo() {
}
//----------------------------------------------------------------------------------------------

void FormInfo::changeEvent(QEvent* event) {
    if(event->type() == QEvent::LanguageChange)
        retranslateUi();
    else
        QWidget::changeEvent(event);
}
//----------------------------------------------------------------------------------------------

void FormInfo::retranslateUi() {
    if(m_item.isDir() == true)
        m_text_type->setText(tr("Директория"));
    else if(m_item.isFile() == true)
        m_text_type->setText(tr("Файл"));

    FormInfoUI::retranslateUi();
}
//----------------------------------------------------------------------------------------------
