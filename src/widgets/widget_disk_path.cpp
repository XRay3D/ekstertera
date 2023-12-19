#include "widget_disk_path.h"
//----------------------------------------------------------------------------------------------

WidgetDiskPath::WidgetDiskPath(QWidget* parent)
    : QWidget(parent) {
    setMinimumHeight(24);
    setMaximumHeight(24);
    m_layout = new QHBoxLayout{this};
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);
    m_layout->setAlignment(Qt::AlignLeft);
}
//----------------------------------------------------------------------------------------------

WidgetDiskPath::~WidgetDiskPath() {
}
//----------------------------------------------------------------------------------------------

void WidgetDiskPath::changePath(const QString& path) {
    qDeleteAll(m_buttons);
    m_buttons.clear();

    QString patrial;
    QStringList parts = path.split("/");

    for(auto&& path: parts) {

        if(path.isEmpty() == true)
            continue;

        QToolButton* button = new QToolButton{this};

        button->setText(path);
        button->setFocusPolicy(Qt::NoFocus);
        button->setVisible(true);

        patrial += path + "/";
        button->setProperty("path", patrial);

        connect(button, &QToolButton::clicked, this, &WidgetDiskPath::button_clicked);

        m_buttons.append(button);
        m_layout->addWidget(button);
    }
}
//----------------------------------------------------------------------------------------------

void WidgetDiskPath::button_clicked() {
    QToolButton* button = static_cast<QToolButton*>(sender());
    QString path = button->property("path").toString();
    emit onPathChangeRequest(path);
}
//----------------------------------------------------------------------------------------------
