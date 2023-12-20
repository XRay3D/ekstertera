#include "form_main_ui.h"
//----------------------------------------------------------------------------------------------
#include "utils/settings.h"

#include <QDockWidget>
#include <QProxyStyle>
//----------------------------------------------------------------------------------------------

class IconnedDockStyle : public QProxyStyle {
    // Q_OBJECT
    QIcon icon_;

public:
    IconnedDockStyle(const QIcon& icon, QStyle* style = 0)
        : QProxyStyle{style}
        , icon_{icon} { }
    ~IconnedDockStyle() override = default;
    void drawControl(ControlElement element, const QStyleOption* option,
        QPainter* painter, const QWidget* widget = 0) const override {
        if(element == QStyle::CE_DockWidgetTitle) {
            // width of the icon
            int width = pixelMetric(QStyle::PM_ToolBarIconSize);
            // margin of title from frame
            int margin = baseStyle()->pixelMetric(QStyle::PM_DockWidgetTitleMargin);

            int k = (option->rect.height() - width) / 2 + margin;
            painter->drawPixmap(k, k, icon_.pixmap(width, width));

            const_cast<QStyleOption*>(option)->rect = option->rect.adjusted(width + margin * 2, 0, 0, 0);
        }
        baseStyle()->drawControl(element, option, painter, widget);
    }
    void setIcon(const QIcon& newIcon) {
        icon_ = newIcon;
    }
};

FormMainUI::FormMainUI()
    : QMainWindow() {
    QIcon icon;
    icon.addFile(":/icons/ekstertera16.png", QSize(16, 16));
    icon.addFile(":/icons/ekstertera24.png", QSize(24, 24));
    icon.addFile(":/icons/ekstertera32.png", QSize(32, 32));
    icon.addFile(":/icons/ekstertera48.png", QSize(48, 48));
    icon.addFile(":/icons/ekstertera64.png", QSize(64, 64));
    icon.addFile(":/icons/ekstertera128.png", QSize(128, 128));
    icon.addFile(":/icons/ekstertera256.png", QSize(256, 256));
    setWindowIcon(icon);

    setMinimumSize(280, 280);

    //
    // центральный виджет
    //

    // m_widget_central = new QWidget{this};
    // m_layout_central = new QVBoxLayout(m_widget_central);

    // m_widget_path = new WidgetDiskPath(this /*m_widget_central*/);
    // m_layout_central->addWidget(m_widget_path);

    m_widget_disk = new WidgetDisk(this /*m_widget_central*/);
    // m_layout_central->addWidget(m_widget_disk);

    // setCentralWidget(m_widget_path /*m_widget_central*/);


    m_explorer_dock = new QDockWidget{this};
    m_tasks_dock = new QDockWidget{this};
    auto setupDockWidget = [](QDockWidget* dw, auto&& name, QWidget* w) {
        dw->setObjectName(name);
        dw->setFeatures(QDockWidget::DockWidgetMovable);
        dw->setWidget(w);
        dw->setAllowedAreas(Qt::BottomDockWidgetArea);
    };
    setupDockWidget(m_explorer_dock, "explorerDock", m_widget_disk->m_explorer);
    setupDockWidget(m_tasks_dock, "tasksDock", m_widget_disk->m_tasks);
    m_tasks_dock->setStyle(m_icon_dock_style = new IconnedDockStyle(QIcon(":icons/green16.png"), m_tasks_dock->style()));

    connect(m_widget_disk->m_tasks, &WidgetTasks::onChangeCount, m_tasks_dock, [this](int count) {
        if(count > 0) {
            m_icon_dock_style->setIcon(QIcon(":icons/yellow16.png"));
            m_tasks_dock->setWindowTitle(tr("Задачи (%1)").arg(count));
        } else {
            m_icon_dock_style->setIcon(QIcon(":icons/green16.png"));
            m_tasks_dock->setWindowTitle(tr("Задачи"));
        }
        m_tasks_dock->update();
    });

    addDockWidget(Qt::BottomDockWidgetArea, m_explorer_dock);
    addDockWidget(Qt::BottomDockWidgetArea, m_tasks_dock);

    //
    // меню
    //

    m_menubar = new QMenuBar{this};

    // верхний ряд меню
    m_menu_file = m_menubar->addMenu("");
    m_menu_edit = m_menubar->addMenu("");
    m_menu_view = m_menubar->addMenu("");
    m_menu_help = m_menubar->addMenu("");

    //
    // меню "Файл"
    //

    // Файл / Выход
    m_menu_file_exit = m_menu_file->addAction(QIcon::fromTheme("application-exit", QIcon(":/icons/tango/application-exit.svg")), "");
    m_menu_file_exit->setShortcut(QKeySequence(QKeySequence::Quit));
    m_menu_file_exit->setMenuRole(QAction::QuitRole);

    //
    // Меню "Правка"
    //

    m_menu_edit->addActions(m_widget_disk->contextMenuActions());

    m_menu_edit->addSeparator();

    m_menu_edit_settings = m_menu_edit->addAction(QIcon::fromTheme("preferences-system", QIcon(":/icons/tango/preferences-system.svg")), "");
    m_menu_edit_settings->setShortcut(QKeySequence(QKeySequence::Preferences));

    //
    // Меню "Вид"
    //

    m_menu_view_refresh = m_menu_view->addAction(QIcon::fromTheme("view-refresh", QIcon(":/icons/tango/view-refresh.svg")), "");
    m_menu_view_refresh->setShortcut(QKeySequence(QKeySequence::Refresh));

    m_menu_view->addSeparator();

    m_menu_view_zoom_in = m_menu_view->addAction(QIcon::fromTheme("zoom-in", QIcon(":/icons/gnome/zoom-in32.png")), "");
    m_menu_view_zoom_in->setShortcut(QKeySequence(QKeySequence::ZoomIn));

    m_menu_view_zoom_out = m_menu_view->addAction(QIcon::fromTheme("zoom-out", QIcon(":/icons/gnome/zoom-out32.png")), "");
    m_menu_view_zoom_out->setShortcut(QKeySequence(QKeySequence::ZoomOut));

    m_menu_view->addSeparator();

    m_menu_view_preview = m_menu_view->addAction("");
    m_menu_view_preview->setCheckable(true);

    //
    // меню "?"
    //

    m_menu_about = m_menu_help->addAction(QIcon::fromTheme("help-about", QIcon(":/icons/gnome/help-about.svg")), "");
    m_menu_about_qt = m_menu_help->addAction("");

    setMenuBar(m_menubar);

    //
    // тулбар
    //

    m_toolbar = addToolBar("");
    m_toolbar->setObjectName("toolBar");
    m_toolbar->setFloatable(false);

    m_toolbar->addAction(m_menu_view_refresh);

    m_action_upload = m_toolbar->addAction(QIcon(":/icons/upload32.png"), "");

    m_action_download = m_toolbar->addAction(QIcon(":/icons/download32.png"), "");
    m_action_download->setEnabled(false);

    m_toolbar->addAction(m_menu_view_zoom_out);
    m_toolbar->addAction(m_menu_view_zoom_in);

    //
    // статусбар
    //

    m_status_bar = new QStatusBar{this};

    m_label_used = new QLabel(m_status_bar);
    m_status_bar->addWidget(m_label_used);

    m_label_total = new QLabel(m_status_bar);
    m_status_bar->addWidget(m_label_total);

    m_label_selected = new QLabel(m_status_bar);
    m_status_bar->addWidget(m_label_selected);
    m_label_selected->setVisible(false);

    setStatusBar(m_status_bar);

    // меню трея
    m_tray_menu = new QMenu{this};
    m_tray_menu_show = m_tray_menu->addAction(QIcon(":/icons/ekstertera16.png"), "");
    m_tray_menu->addSeparator();
    m_tray_menu->addAction(m_menu_file_exit);

    // иконка трея
    m_tray_icon = new WidgetTrayIcon(this);
    m_tray_icon->setContextMenu(m_tray_menu);
    m_tray_icon->setIcon(icon);
    m_tray_icon->setVisible(EteraSettings::instance()->closeToTray());

    // локализация
    retranslateUi();

    // восстановление layout
    restore();
}
//----------------------------------------------------------------------------------------------

void FormMainUI::closeEvent(QCloseEvent* event) {
    // сохранение layout
    if(m_tray_icon->isVisible() == true && EteraSettings::instance()->closeToTray() == true) {
        hide();
        event->ignore();
    } else {
        save();
        event->accept();
    }
}
//----------------------------------------------------------------------------------------------

void FormMainUI::changeEvent(QEvent* event) {
    if(event->type() == QEvent::LanguageChange)
        retranslateUi();
    else
        QWidget::changeEvent(event);
}
//----------------------------------------------------------------------------------------------

void FormMainUI::retranslateUi() {
    setWindowTitle(tr("Ekstertera"));

    m_menu_file->setTitle(tr("&Файл"));

    m_menu_file_exit->setText(tr("Выход"));

    m_menu_edit->setTitle(tr("&Правка"));

    m_menu_edit_settings->setText(tr("Параметры"));

    m_menu_view->setTitle(tr("&Вид"));

    m_menu_view_refresh->setText(tr("Обновить"));
    m_menu_view_zoom_in->setText(tr("Увеличить"));
    m_menu_view_zoom_out->setText(tr("Уменьшить"));
    m_menu_view_preview->setText(tr("Предпросмотр"));

    m_menu_help->setTitle(tr("&?"));

    m_menu_about->setText(tr("О программе"));
    m_menu_about_qt->setText(tr("О Qt"));

    m_toolbar->setWindowTitle(tr("Панель инструментов"));

    m_action_upload->setText(tr("Загрузить на Яндекс.Диск"));
    m_action_download->setText(tr("Загрузить с Яндекс.Диска"));

    m_label_used->setToolTip(tr("Используется"));
    m_label_total->setToolTip(tr("Всего"));

    m_tray_menu_show->setText(tr("Показать"));

    m_explorer_dock->setWindowTitle(tr("Проводник"));
    m_tasks_dock->setWindowTitle(tr("Задачи"));

#ifdef ETERA_CUSTOM_TRAY_ICON
    m_tray_icon->retranslateUi();
#endif
}
//----------------------------------------------------------------------------------------------

void FormMainUI::save() {
    QSettings settings;
    settings.setValue("layout/main", saveGeometry());
    settings.setValue("state/main", saveState());
    settings.setValue("app/zoom", m_widget_disk->zoomFactor());
    settings.setValue("app/preview", m_menu_view_preview->isChecked());
}
//----------------------------------------------------------------------------------------------

void FormMainUI::restore() {
    QSettings settings;

    restoreGeometry(settings.value("layout/main").toByteArray());
    restoreState(settings.value("state/main").toByteArray());

    int zoom = m_widget_disk->setZoomFactor(settings.value("app/zoom", -1).toInt());
    if(zoom < 0)
        m_menu_view_zoom_out->setEnabled(false);
    else if(zoom > 0)
        m_menu_view_zoom_in->setEnabled(false);

    m_menu_view_preview->setChecked(settings.value("app/preview", false).toBool());
    m_widget_disk->setPreviewMode(m_menu_view_preview->isChecked());
}
//----------------------------------------------------------------------------------------------
