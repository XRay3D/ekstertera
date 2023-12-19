#include "widget_disk.h"
//----------------------------------------------------------------------------------------------
#include "widget_disk_item_delegate.h"
//----------------------------------------------------------------------------------------------
#include "utils/clipboard.h"
#include "utils/icon.h"
#include "utils/settings.h"
//----------------------------------------------------------------------------------------------
#include "forms/form_info.h"
//----------------------------------------------------------------------------------------------

WidgetDisk::WidgetDisk(QWidget* parent)
    : QTabWidget(parent) {
    m_preview_mode = false;

    m_message_box_active = false;

    m_put_activity_limit = 8 /* TODO: #3 */;
    m_get_activity_limit = 8 /* TODO: #3 */;

    m_explorer = new QListWidget{this};

    m_explorer->setWrapping(true);
    m_explorer->setResizeMode(QListView::Adjust);
    m_explorer->setContextMenuPolicy(Qt::CustomContextMenu);
    m_explorer->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_explorer->setSortingEnabled(true);
    m_explorer->setItemDelegate(new WidgetDiskItemDelegate(m_explorer));

    setZoomFactor(EteraIconProvider::instance()->defaultIconSizeIndex());

    m_tasks = new WidgetTasks(this);

    setAcceptDrops(true);

    addTab(m_explorer, "");
    addTab(m_tasks, QIcon(":icons/green16.png"), "");

    int size = EteraIconProvider::instance()->maxIconSize();
    m_preview_arg = QString("%1x%2").arg(size).arg(size);

    // контекстное меню
    m_menu = new QMenu(m_explorer);

    m_menu_open = m_menu->addAction(QIcon::fromTheme("folder-open", QIcon(":/icons/tango/folder-open.svg")), "");
    m_menu_open->setShortcut(QKeySequence("Enter"));

    m_menu->addSeparator();

    m_menu_new = m_menu->addAction(QIcon::fromTheme("folder-new", QIcon(":/icons/tango/folder-new.svg")), "");
    m_menu_new->setShortcut(QKeySequence(QKeySequence::New));

    m_menu->addSeparator();

    m_menu_cut = m_menu->addAction(QIcon::fromTheme("edit-cut", QIcon(":/icons/tango/edit-cut.svg")), "");
    m_menu_cut->setShortcut(QKeySequence(QKeySequence::Cut));

    m_menu_copy = m_menu->addAction(QIcon::fromTheme("edit-copy", QIcon(":/icons/tango/edit-copy.svg")), "");
    m_menu_copy->setShortcut(QKeySequence(QKeySequence::Copy));

    m_menu_paste = m_menu->addAction(QIcon::fromTheme("edit-paste", QIcon(":/icons/tango/edit-paste.svg")), "");
    m_menu_paste->setShortcut(QKeySequence(QKeySequence::Paste));

    m_menu->addSeparator();

    m_menu_delete = m_menu->addAction(QIcon::fromTheme("edit-delete", QIcon(":/icons/tango/edit-delete.svg")), "");
    m_menu_delete->setShortcut(QKeySequence(QKeySequence::Delete));

    m_menu->addSeparator();

    m_menu_rename = m_menu->addAction("");
    m_menu_rename->setShortcut(QKeySequence("F2"));

    m_menu->addSeparator();

    m_menu_share = m_menu->addAction(QIcon::fromTheme("emblem-shared", QIcon(":/icons/gnome/emblem-shared16.png")), "");
    m_menu_revoke = m_menu->addAction(QIcon::fromTheme("document-revert", QIcon(":/icons/gnome/document-revert16.png")), "");

    m_menu->addSeparator();

    m_menu_info = m_menu->addAction(QIcon::fromTheme("document-properties", QIcon(":/icons/tango/document-properties.svg")), "");

    connect(m_menu_open, &QAction::triggered, this, &WidgetDisk::menu_open_triggered);
    connect(m_menu_new, &QAction::triggered, this, &WidgetDisk::menu_new_triggered);
    connect(m_menu_cut, &QAction::triggered, this, &WidgetDisk::menu_cut_triggered);
    connect(m_menu_copy, &QAction::triggered, this, &WidgetDisk::menu_copy_triggered);
    connect(m_menu_paste, &QAction::triggered, this, &WidgetDisk::menu_paste_triggered);
    connect(m_menu_delete, &QAction::triggered, this, &WidgetDisk::menu_delete_triggered);
    connect(m_menu_rename, &QAction::triggered, this, &WidgetDisk::menu_rename_triggered);
    connect(m_menu_share, &QAction::triggered, this, &WidgetDisk::menu_share_triggered);
    connect(m_menu_revoke, &QAction::triggered, this, &WidgetDisk::menu_revoke_triggered);
    connect(m_menu_info, &QAction::triggered, this, &WidgetDisk::menu_info_triggered);

    // события виджета проводника диска
    connect(m_explorer, &QListWidget::itemSelectionChanged, this, &WidgetDisk::on_item_selection_changed);
    connect(m_explorer, &QListWidget::itemActivated, this, &WidgetDisk::item_activated);
    connect(m_explorer, &QListWidget::customContextMenuRequested, this, &WidgetDisk::custom_context_menu_requested);
    connect(m_explorer->itemDelegate(), &QAbstractItemDelegate::closeEditor, this, &WidgetDisk::item_end_edit);

    // события виджета асинхронных задач
    connect(m_tasks, &WidgetTasks::onChangeCount, this, &WidgetDisk::widget_tasks_on_change_count);

    // локализация
    retranslateUi();

    // включение / выключение пунктов контекстного меню
    on_item_selection_changed();

    // таймер отложенных сигналов / задач
    m_emit_timer = new QTimer{this};
    connect(m_emit_timer, &QTimer::timeout, this, &WidgetDisk::emit_delayed_signals);
    m_emit_timer->start(10000); /* TODO: #2 */
}
//----------------------------------------------------------------------------------------------

WidgetDisk::~WidgetDisk() {
}
//----------------------------------------------------------------------------------------------

void WidgetDisk::retranslateUi() {
    m_menu_open->setText(tr("Открыть"));
    m_menu_new->setText(tr("Создать"));
    m_menu_cut->setText(tr("Вырезать"));
    m_menu_copy->setText(tr("Копировать"));
    m_menu_paste->setText(tr("Вставить"));
    m_menu_delete->setText(tr("Удалить"));
    m_menu_rename->setText(tr("Переименовать"));
    m_menu_share->setText(tr("Открыть доступ"));
    m_menu_revoke->setText(tr("Закрыть доступ"));
    m_menu_info->setText(tr("Свойства"));

    setTabText(0, tr("Проводник"));
    setTabText(1, tr("Задачи"));

    //
    // информационные сообщения
    //

    ERROR_MESSAGE = tr("Ошибка!");
    ATTENTION_MESSAGE = tr("Внимание!");
    ERROR_MESSAGE_LS = tr("Ошибка чтения %1:\n%2");
    START_MESSAGE_LS = tr("Чтение %1");
    ERROR_MESSAGE_MKDIR = tr("Ошибка создания %1:\n%2");
    START_MESSAGE_MKDIR_CAPTION = tr("Создать директорию");
    START_MESSAGE_MKDIR_TEXT = tr("Введите имя новой директории");
    START_MESSAGE_MKDIR_VALUE = tr("Новая папка");
    ERROR_MESSAGE_MKDIR_ALREADY_EXISTS = tr("Директория %1 уже существует");
    START_MESSAGE_MKDIR = tr("Создание %1");
    ERROR_MESSAGE_STAT = tr("Ошибка чтения информации о %1:\n%2");
    START_MESSAGE_STAT = tr("Чтение информации о %1");
    ROOT_MESSAGE_CP = tr("Копирование");
    ERROR_MESSAGE_CP = tr("Ошибка копирования %1 в %2:\n%3");
    ERROR_MESSAGE_CP_ALREADY_EXISTS = tr("Ошибка копирования %1:\n%2 уже существует");
    START_MESSAGE_CP = tr("Копирование %1 в %2");
    ROOT_MESSAGE_MV = tr("Перемещение");
    ERROR_MESSAGE_MV = tr("Ошибка перемещения %1 в %2:\n%3");
    ERROR_MESSAGE_MV_ALREADY_EXISTS = tr("Ошибка перемещения %1:\n%2 уже существует");
    START_MESSAGE_MV = tr("Перемещение %1 в %2");
    ERROR_MESSAGE_CPMV_SAME = tr("Источник и приемник совпадают");
    ROOT_MESSAGE_RM = tr("Удаление");
    ASK_DELETE_MESSAGE = tr("Вы уверены, что хотите удалить выбранные элементы?");
    ERROR_MESSAGE_RM = tr("Ошибка удаления %1:\n%2");
    START_MESSAGE_RM = tr("Удаление %1");
    ERROR_MESSAGE_RENAME_INVALID_CHAR = tr("Недопустимый символ \"%1\" в имени");
    ERROR_MESSAGE_RENAME_IVALID_NAME = tr("Недопустимое имя");
    ERROR_MESSAGE_RENAME = tr("Ошибка переименования %1 в %2:\n%3");
    ERROR_MESSAGE_RENAME_ALREADY_EXISTS = tr("Ошибка переименования %1:\n%2 же существует");
    START_MESSAGE_RENAME = tr("Переименование %1 в %2");
    ROOT_MESSAGE_PUBLISH = tr("Открытие доступа");
    ERROR_MESSAGE_PUBLISH = tr("Ошибка открытия доступа к %1:\n%2");
    START_MESSAGE_PUBLISH = tr("Открытие доступа к %1");
    ROOT_MESSAGE_UNPUBLISH = tr("Закрытие доступа");
    ERROR_MESSAGE_UNPUBLISH = tr("Ошибка закрытия доступа к %1:\n%2");
    START_MESSAGE_UNPUBLISH = tr("Закрытие доступа к %1");
    ROOT_MESSAGE_UPLOAD = tr("Отправка на Диск");
    ERROR_MESSAGE_UPLOAD = tr("Ошибка отправки %1 в %2:\n%3");
    START_MESSAGE_UPLOAD = tr("Отправка %1 в %2");
    START_MESSAGE_UPLOAD_CAPTION = tr("Файл уже существует!");
    START_MESSAGE_UPLOAD_TEXT = tr("Файл %1 уже существует, перезаписать?");
    ROOT_MESSAGE_DOWNLOAD = tr("Загрузка с Диска");
    ERROR_MESSAGE_DOWNLOAD = tr("Ошибка загрузки %1 в %2:\n%3");
    START_MESSAGE_DOWNLOAD = tr("Загрузка %1 в %2");
    START_MESSAGE_DOWNLOAD_CAPTION = tr("Файл уже существует!");
    START_MESSAGE_DOWNLOAD_TEXT = tr("Файл %1 уже существует, перезаписать?");
}
//----------------------------------------------------------------------------------------------

void WidgetDisk::wheelEvent(QWheelEvent* event) {
    m_explorer->horizontalScrollBar()->event(event);
}
//----------------------------------------------------------------------------------------------

EteraAPI* WidgetDisk::createAPI(quint64 id) {
    EteraAPI* api = new EteraAPI(this, id);
    api->setToken(EteraSettings::instance()->token());
    return api;
}
//----------------------------------------------------------------------------------------------

void WidgetDisk::releaseAPI(EteraAPI* api) {
    if(api != nullptr) {
        m_tasks->removeTask(api->id());
        api->deleteLater();
    }
}
//----------------------------------------------------------------------------------------------

void WidgetDisk::updateBufferList(bool copy_mode) {
    QList<QListWidgetItem*> selected = m_explorer->selectedItems();

    if(selected.isEmpty() == true)
        return;

    EteraClipboard* clipboard = EteraClipboard::instance();

    clipboard->clear();

    if(copy_mode == true)
        clipboard->setCopyMode();
    else
        clipboard->setCutMode();

    for(auto&& item: selected)
        clipboard->append(*((static_cast<WidgetDiskItem*>(item))->item()));
}
//----------------------------------------------------------------------------------------------

WidgetDiskItem* WidgetDisk::findByPath(const QString& path) {
    // оптимизировать O(n) при необходимости
    for(int i = 0; i < m_explorer->count(); ++i) {
        WidgetDiskItem* witem = static_cast<WidgetDiskItem*>(m_explorer->item(i));
        const EteraItem* eitem = witem->item();
        if(eitem->path() == path)
            return witem;
    }

    return nullptr;
}
//----------------------------------------------------------------------------------------------

void WidgetDisk::removeByPath(const QString& path) {
    // оптимизировать O(n) при необходимости
    for(int i = 0; i < m_explorer->count(); ++i) {
        WidgetDiskItem* witem = static_cast<WidgetDiskItem*>(m_explorer->item(i));
        const EteraItem* eitem = witem->item();
        if(eitem->path() == path) {
            m_explorer->takeItem(i);
            delete witem;
            break;
        }
    }
}
//----------------------------------------------------------------------------------------------

QString WidgetDisk::remoteBasename(const QString& path) {
    int idx = path.lastIndexOf('/');
    if(idx == -1)
        return path;

    return path.right(path.length() - idx - 1);
}
//----------------------------------------------------------------------------------------------

QString WidgetDisk::localBasename(const QString& path) {
#ifndef ETERA_WS_WIN
    return remoteBasename(path);
#else
    // под windows могут использоваться оба разделителя в зависимости от контекста
    int idx = path.lastIndexOf('\\');
    if(idx == -1)
        return remoteBasename(path);

    return path.right(path.length() - idx - 1);
#endif
}
//----------------------------------------------------------------------------------------------

void WidgetDisk::widget_tasks_on_change_count(int count) {
    if(count > 0) {
        setTabIcon(1, QIcon(":icons/yellow16.png"));
        setTabText(1, tr("Задачи (%1)").arg(count));
    } else {
        setTabIcon(1, QIcon(":icons/green16.png"));
        setTabText(1, tr("Задачи"));
    }
}
//----------------------------------------------------------------------------------------------

void WidgetDisk::on_item_selection_changed() {
    if(m_explorer->selectedItems().count() == 0)
        emit onChangePossibleActions(false);
    else
        emit onChangePossibleActions(true);

    QList<QListWidgetItem*> selected = m_explorer->selectedItems();

    bool e = (selected.isEmpty() == false);
    m_menu_cut->setEnabled(e);
    m_menu_copy->setEnabled(e);
    m_menu_delete->setEnabled(e);

    if(selected.count() == 1) {
        WidgetDiskItem* witem = static_cast<WidgetDiskItem*>(selected[0]);
        const EteraItem* eitem = witem->item();
        if(eitem->isDir() == true)
            m_menu_open->setEnabled(true);
        else
            m_menu_open->setEnabled(false);

        m_menu_rename->setEnabled(true);
        m_menu_info->setEnabled(true);
    } else {
        m_menu_open->setEnabled(false);
        m_menu_rename->setEnabled(false);
        m_menu_info->setEnabled(false);
    }

    int files = 0;
    int dirs = 0;
    quint64 size = 0;

    bool can_share = false;
    bool can_revoke = false;

    for(auto&& item: selected) {
        WidgetDiskItem* witem = static_cast<WidgetDiskItem*>(item);
        const EteraItem* eitem = witem->item();

        if(eitem->isPublic() == true)
            can_revoke = true;
        else
            can_share = true;

        if(eitem->isDir() == true)
            dirs++;
        else if(eitem->isFile() == true) {
            files++;
            size += eitem->size();
        }
    }

    m_menu_share->setEnabled(can_share);
    m_menu_revoke->setEnabled(can_revoke);

    EteraClipboard* clipboard = EteraClipboard::instance();

    m_menu_paste->setEnabled(clipboard->isEmpty() == false);

    emit onSelectionChanged(files, dirs, size);
}
//----------------------------------------------------------------------------------------------

int WidgetDisk::setZoomFactor(int factor) {
    const QList<int>* sizes = EteraIconProvider::instance()->iconSizes();

    if(factor < 0 || factor >= sizes->count())
        factor = EteraIconProvider::instance()->defaultIconSizeIndex();

    m_icon_size_index = factor;

    int size = sizes->at(m_icon_size_index);

    m_explorer->setIconSize(QSize(size, size));

    if(m_icon_size_index == 0)
        return -1;
    else if(m_icon_size_index == sizes->count() - 1)
        return 1;

    return 0;
}
//----------------------------------------------------------------------------------------------

bool WidgetDisk::zoomIn() {
    const QList<int>* sizes = EteraIconProvider::instance()->iconSizes();

    if(m_icon_size_index < sizes->count() - 1)
        m_icon_size_index++;

    int size = sizes->at(m_icon_size_index);

    m_explorer->setIconSize(QSize(size, size));

    return (m_icon_size_index == sizes->count() - 1 ? false : true);
}
//----------------------------------------------------------------------------------------------

bool WidgetDisk::zoomOut() {
    const QList<int>* sizes = EteraIconProvider::instance()->iconSizes();

    if(m_icon_size_index != 0)
        m_icon_size_index--;

    int size = sizes->at(m_icon_size_index);

    m_explorer->setIconSize(QSize(size, size));

    return (m_icon_size_index == 0 ? false : true);
}
//----------------------------------------------------------------------------------------------

void WidgetDisk::setPreviewMode(bool mode) {
    m_preview_mode = mode;

    if(m_preview_mode == false)
        EteraIconProvider::instance()->cancelPreview();

    for(int i = 0; i < m_explorer->count(); ++i) {
        WidgetDiskItem* item = static_cast<WidgetDiskItem*>(m_explorer->item(i));
        item->update(m_preview_mode);
    }
}
//----------------------------------------------------------------------------------------------

void WidgetDisk::changePath(const QString& path) {
    m_explorer->setCursor(Qt::BusyCursor);

    m_path = "";
    m_required_path = path;

    m_explorer->clear();

    QString _path = path;
    if(_path.endsWith("/") == false) {
        _path += "/";
        m_required_path += "/";
    }

    EteraAPI* api = createAPI();

    ETERA_API_TASK_LS(api, task_on_ls_success, task_on_ls_error);

    m_tasks->addTask(api->id(), START_MESSAGE_LS.arg(_path));

    api->ls(_path, m_preview_arg, true);
}
//----------------------------------------------------------------------------------------------

void WidgetDisk::task_on_ls_error(EteraAPI* api) {
    if(api->lastErrorCode() == QNetworkReply::OperationCanceledError) {
        m_explorer->setCursor(Qt::ArrowCursor);
        releaseAPI(api);
        return;
    }

    if(api->canRetry() == true) {
        delayTask(slot_task_on_ls_error, api);
        return;
    }

    if(messageBoxLocked() == true) {
        delayEmit(slot_task_on_ls_error, api);
        return;
    }

    messageBoxLock();
    QMessageBox::StandardButton reply = QMessageBox::critical(this, ERROR_MESSAGE, ERROR_MESSAGE_LS.arg(api->path()).arg(api->lastErrorMessage()), QMessageBox::Retry | QMessageBox::Ignore);
    messageBoxUnlock();

    if(api->deleted() == true) {
        m_explorer->setCursor(Qt::ArrowCursor);
        return;
    }

    if(reply == QMessageBox::Retry)
        api->ls(api->path(), api->preview(), api->crop(), api->offset(), api->limit());
    else if(reply == QMessageBox::Ignore) {
        m_explorer->setCursor(Qt::ArrowCursor);
        releaseAPI(api);
    }
}
//----------------------------------------------------------------------------------------------

void WidgetDisk::task_on_ls_success(EteraAPI* api, const EteraItemList& list, quint64 limit) {
    // если загружался большой список, но в это время сменили текущий путь - останавливаем предыдущую работу
    if(api->path() != m_required_path) {
        releaseAPI(api);
        return;
    }

    // добавление элементов
    for(auto&& item: list)
        new WidgetDiskItem(m_explorer, item, m_preview_mode);

    // проверка необходимости остановки
    if((quint64)list.count() < limit) {
        m_path = api->path();

        m_explorer->setCursor(Qt::ArrowCursor);

        emit onPathChanged(api->path());

        releaseAPI(api);
    } else {
        quint64 offset = api->offset() + limit;
        api->ls(api->path(), api->preview(), api->crop(), offset, limit);
    }
}
//----------------------------------------------------------------------------------------------

void WidgetDisk::item_activated(QListWidgetItem* item) {
    WidgetDiskItem* witem = static_cast<WidgetDiskItem*>(item);
    const EteraItem* eitem = witem->item();

    if(eitem->isDir() == false)
        return;

    QString path = eitem->path();

    changePath(path);
}
//----------------------------------------------------------------------------------------------

void WidgetDisk::custom_context_menu_requested(const QPoint& pos) {
    if(m_path.isEmpty() == true)
        return;

    m_menu->exec(m_explorer->viewport()->mapToGlobal(pos));
}
//----------------------------------------------------------------------------------------------

void WidgetDisk::menu_open_triggered() {
    WidgetDiskItem* witem = static_cast<WidgetDiskItem*>(m_explorer->currentItem());
    if(witem == nullptr)
        return;

    const EteraItem* eitem = witem->item();

    if(eitem->isDir() == true)
        changePath(eitem->path());
}
//----------------------------------------------------------------------------------------------

void WidgetDisk::menu_new_triggered() {
    if(m_path.isEmpty() == true)
        return;

    bool ok;
    QString value = QInputDialog::getText(this, START_MESSAGE_MKDIR_CAPTION, START_MESSAGE_MKDIR_TEXT, QLineEdit::Normal, START_MESSAGE_MKDIR_VALUE, &ok);
    if(ok == false || value.isEmpty() == true)
        return;

    QString path = m_path + value;

    EteraAPI* api = createAPI();

    ETERA_API_TASK_MKDIR(api, task_on_mkdir_success, task_on_mkdir_error);

    m_tasks->addTask(api->id(), START_MESSAGE_MKDIR.arg(path));

    api->mkdir(path);
}
//----------------------------------------------------------------------------------------------

void WidgetDisk::task_on_mkdir_error(EteraAPI* api) {
    if(api->lastErrorCode() == QNetworkReply::OperationCanceledError) {
        releaseAPI(api);
        return;
    }

    if(api->canRetry() == true) {
        delayTask(slot_task_on_mkdir_error, api);
        return;
    }

    if(messageBoxLocked() == true) {
        delayEmit(slot_task_on_mkdir_error, api);
        return;
    }

    messageBoxLock();
    QMessageBox::StandardButton reply;
    if(api->lastErrorCode() == 409) // CONFLICT, директория уже существует
        reply = QMessageBox::warning(this, ATTENTION_MESSAGE, ERROR_MESSAGE_MKDIR_ALREADY_EXISTS.arg(api->path()));
    else
        reply = QMessageBox::critical(this, ERROR_MESSAGE, ERROR_MESSAGE_MKDIR.arg(api->path()).arg(api->lastErrorMessage()), QMessageBox::Retry | QMessageBox::Ignore);
    messageBoxUnlock();

    if(api->deleted() == true)
        return;

    if(reply == QMessageBox::Retry)
        api->mkdir(api->path());
    else if(reply == QMessageBox::Ignore || reply == QMessageBox::Ok)
        releaseAPI(api);
}
//----------------------------------------------------------------------------------------------

void WidgetDisk::task_on_mkdir_success(EteraAPI* api) {
    ETERA_API_TASK_STAT(api, task_on_mkdir_stat_success, task_on_mkdir_stat_error);

    m_tasks->addTask(api->id(), START_MESSAGE_STAT.arg(api->path()));

    api->stat(api->path());
}
//----------------------------------------------------------------------------------------------

void WidgetDisk::task_on_mkdir_stat_error(EteraAPI* api) {
    if(api->lastErrorCode() == QNetworkReply::OperationCanceledError) {
        releaseAPI(api);
        return;
    }

    if(api->canRetry() == true) {
        delayTask(slot_task_on_mkdir_stat_error, api);
        return;
    }

    if(messageBoxLocked() == true) {
        delayEmit(slot_task_on_mkdir_stat_error, api);
        return;
    }

    messageBoxLock();
    QMessageBox::StandardButton reply = QMessageBox::critical(this, ERROR_MESSAGE, ERROR_MESSAGE_STAT.arg(api->path()).arg(api->lastErrorMessage()), QMessageBox::Retry | QMessageBox::Ignore);
    messageBoxUnlock();

    if(api->deleted() == true)
        return;

    if(reply == QMessageBox::Retry)
        api->stat(api->path());
    else if(reply == QMessageBox::Ignore)
        releaseAPI(api);
}
//----------------------------------------------------------------------------------------------

void WidgetDisk::task_on_mkdir_stat_success(EteraAPI* api, const EteraItem& item) {
    if(item.parentPath() == m_path)
        m_explorer->setCurrentItem(new WidgetDiskItem(m_explorer, item, m_preview_mode), QItemSelectionModel::ClearAndSelect);

    releaseAPI(api);
}
//----------------------------------------------------------------------------------------------

void WidgetDisk::menu_cut_triggered() {
    updateBufferList(false);
}
//----------------------------------------------------------------------------------------------

void WidgetDisk::menu_copy_triggered() {
    updateBufferList(true);
}
//----------------------------------------------------------------------------------------------

void WidgetDisk::menu_paste_triggered() {
    EteraClipboard* clipboard = EteraClipboard::instance();

    if(clipboard->isEmpty() == true || m_path.isEmpty() == true)
        return;

    quint64 parent = 0;
    if(clipboard->count() > 1) {
        parent = EteraAPI::nextID();
        if(clipboard->copyMode() == true)
            m_tasks->addTask(parent, ROOT_MESSAGE_CP);
        else
            m_tasks->addTask(parent, ROOT_MESSAGE_MV);
    }

    for(auto&& src: *clipboard) {
        QString dst = m_path + src.name();

        // нет смысла перемещать само в себя
        if(src.path() == dst) {
            QMessageBox::warning(this, ATTENTION_MESSAGE, ERROR_MESSAGE_CPMV_SAME);
            return;
        }

        EteraAPI* api = createAPI();

        if(clipboard->copyMode() == true) {
            ETERA_API_TASK_CP(api, task_on_copy_paste_success, task_on_copy_paste_error);

            m_tasks->addChildTask(parent, api->id(), START_MESSAGE_CP.arg(src.path()).arg(dst));

            api->cp(src.path(), dst, false);
        } else {
            ETERA_API_TASK_MV(api, task_on_cut_paste_success, task_on_cut_paste_error);

            m_tasks->addChildTask(parent, api->id(), START_MESSAGE_MV.arg(src.path()).arg(dst));

            api->mv(src.path(), dst, false);
        }
    }
}
//----------------------------------------------------------------------------------------------

void WidgetDisk::task_on_copy_paste_error(EteraAPI* api) {
    if(api->lastErrorCode() == QNetworkReply::OperationCanceledError) {
        releaseAPI(api);
        return;
    }

    if(api->canRetry() == true) {
        delayTask(slot_task_on_copy_paste_error, api);
        return;
    }

    if(messageBoxLocked() == true) {
        delayEmit(slot_task_on_copy_paste_error, api);
        return;
    }

    messageBoxLock();
    QMessageBox::StandardButton reply;
    if(api->lastErrorCode() == 409) // CONFLICT, объект уже существует
        reply = QMessageBox::warning(this, ATTENTION_MESSAGE, ERROR_MESSAGE_CP_ALREADY_EXISTS.arg(api->source()).arg(api->target()));
    else
        reply = QMessageBox::critical(this, ERROR_MESSAGE, ERROR_MESSAGE_CP.arg(api->source()).arg(api->target()).arg(api->lastErrorMessage()), QMessageBox::Retry | QMessageBox::Ignore);
    messageBoxUnlock();

    if(api->deleted() == true)
        return;

    // TODO: обработать асинхронную ошибку, т.к. копирование могло быть все же успешным
    if(reply == QMessageBox::Retry)
        api->cp(api->source(), api->target(), false);
    else if(reply == QMessageBox::Ignore || reply == QMessageBox::Ok)
        releaseAPI(api);
}
//----------------------------------------------------------------------------------------------

void WidgetDisk::task_on_copy_paste_success(EteraAPI* api) {
    EteraClipboard::instance()->removeByPath(api->source());

    ETERA_API_TASK_STAT(api, task_on_copy_cut_paste_stat_success, task_on_copy_cut_paste_stat_error);

    m_tasks->addChildTask(api->parentID(), api->id(), START_MESSAGE_STAT.arg(api->target()));

    api->stat(api->target(), m_preview_arg, true);
}
//----------------------------------------------------------------------------------------------

void WidgetDisk::task_on_cut_paste_error(EteraAPI* api) {
    if(api->lastErrorCode() == QNetworkReply::OperationCanceledError) {
        releaseAPI(api);
        return;
    }

    if(api->canRetry() == true) {
        delayTask(slot_task_on_cut_paste_error, api);
        return;
    }

    if(messageBoxLocked() == true) {
        delayEmit(slot_task_on_cut_paste_error, api);
        return;
    }

    messageBoxLock();
    QMessageBox::StandardButton reply;
    if(api->lastErrorCode() == 409) // CONFLICT, объект уже существует
        reply = QMessageBox::warning(this, ATTENTION_MESSAGE, ERROR_MESSAGE_MV_ALREADY_EXISTS.arg(api->source()).arg(api->target()));
    else
        reply = QMessageBox::critical(this, ERROR_MESSAGE, ERROR_MESSAGE_MV.arg(api->source()).arg(api->target()).arg(api->lastErrorMessage()), QMessageBox::Retry | QMessageBox::Ignore);
    messageBoxUnlock();

    if(api->deleted() == true)
        return;

    // TODO: обработать асинхронную ошибку, т.к. вставка могла быть все же успешной
    if(reply == QMessageBox::Retry)
        api->mv(api->source(), api->target(), false);
    else if(reply == QMessageBox::Ignore || reply == QMessageBox::Ok)
        releaseAPI(api);
}
//----------------------------------------------------------------------------------------------

void WidgetDisk::task_on_cut_paste_success(EteraAPI* api) {
    EteraClipboard::instance()->removeByPath(api->source());

    ETERA_API_TASK_STAT(api, task_on_copy_cut_paste_stat_success, task_on_copy_cut_paste_stat_error);

    m_tasks->addChildTask(api->parentID(), api->id(), START_MESSAGE_STAT.arg(api->target()));

    api->stat(api->target(), m_preview_arg, true);
}
//----------------------------------------------------------------------------------------------

void WidgetDisk::task_on_copy_cut_paste_stat_error(EteraAPI* api) {
    if(api->lastErrorCode() == QNetworkReply::OperationCanceledError) {
        releaseAPI(api);
        return;
    }

    if(api->canRetry() == true) {
        delayTask(slot_task_on_copy_cut_paste_stat_error, api);
        return;
    }

    if(messageBoxLocked() == true) {
        delayEmit(slot_task_on_copy_cut_paste_stat_error, api);
        return;
    }

    messageBoxLock();
    QMessageBox::StandardButton reply = QMessageBox::critical(this, ERROR_MESSAGE, ERROR_MESSAGE_STAT.arg(api->target()).arg(api->lastErrorMessage()));
    messageBoxUnlock();

    if(api->deleted() == true)
        return;

    if(reply == QMessageBox::Retry)
        api->stat(api->path(), m_preview_arg, true);
    else if(reply == QMessageBox::Ignore)
        releaseAPI(api);
}
//----------------------------------------------------------------------------------------------

void WidgetDisk::task_on_copy_cut_paste_stat_success(EteraAPI* api, const EteraItem& item) {
    if(item.parentPath() == m_path)
        m_explorer->setCurrentItem(new WidgetDiskItem(m_explorer, item, m_preview_mode), QItemSelectionModel::ClearAndSelect);

    releaseAPI(api);
}
//----------------------------------------------------------------------------------------------

void WidgetDisk::menu_delete_triggered() {
    QList<QListWidgetItem*> selected = m_explorer->selectedItems();

    if(selected.count() == 0)
        return;

    if(QMessageBox::question(this, ATTENTION_MESSAGE, ASK_DELETE_MESSAGE, QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes)
        return;

    quint64 parent = 0;
    if(selected.count() > 1) {
        parent = EteraAPI::nextID();
        m_tasks->addTask(parent, ROOT_MESSAGE_RM);
    }

    for(auto&& item: selected) {
        WidgetDiskItem* witem = static_cast<WidgetDiskItem*>(item);
        const EteraItem* eitem = witem->item();

        EteraAPI* api = createAPI();

        ETERA_API_TASK_RM(api, task_on_rm_success, task_on_rm_error);

        m_tasks->addChildTask(parent, api->id(), START_MESSAGE_RM.arg(eitem->path()));

        api->rm(eitem->path(), true);
    }
}
//----------------------------------------------------------------------------------------------

void WidgetDisk::task_on_rm_error(EteraAPI* api) {
    if(api->lastErrorCode() == QNetworkReply::OperationCanceledError) {
        releaseAPI(api);
        return;
    }

    if(api->canRetry() == true) {
        delayTask(slot_task_on_rm_error, api);
        return;
    }

    // если объект (уже?) не существует, то операция успешна
    if(api->lastErrorCode() == 404) {
        removeByPath(api->path());
        EteraClipboard::instance()->removeByPath(api->path());
        releaseAPI(api);
        return;
    }

    if(messageBoxLocked() == true) {
        delayEmit(slot_task_on_rm_error, api);
        return;
    }

    messageBoxLock();
    QMessageBox::StandardButton reply = QMessageBox::critical(this, ERROR_MESSAGE, ERROR_MESSAGE_RM.arg(api->path()).arg(api->lastErrorMessage()), QMessageBox::Retry | QMessageBox::Ignore);
    messageBoxUnlock();

    if(api->deleted() == true)
        return;

    if(reply == QMessageBox::Retry)
        api->rm(api->path(), true);
    else if(reply == QMessageBox::Ignore)
        releaseAPI(api);
}
//----------------------------------------------------------------------------------------------

void WidgetDisk::task_on_rm_success(EteraAPI* api) {
    removeByPath(api->path());

    EteraClipboard::instance()->removeByPath(api->path());

    releaseAPI(api);
}
//----------------------------------------------------------------------------------------------

void WidgetDisk::menu_rename_triggered() {
    QList<QListWidgetItem*> selected = m_explorer->selectedItems();

    if(selected.count() == 1) {
        WidgetDiskItem* witem = static_cast<WidgetDiskItem*>(m_explorer->currentItem());
        witem->setFlags(witem->flags() | Qt::ItemIsEditable);
        m_explorer->editItem(witem);
    }
}
//----------------------------------------------------------------------------------------------

void WidgetDisk::item_end_edit(QWidget* editor, QAbstractItemDelegate::EndEditHint /*hint*/) {
    WidgetDiskItem* witem = static_cast<WidgetDiskItem*>(m_explorer->currentItem());
    if(witem == nullptr)
        return;

    witem->setFlags(witem->flags() ^ Qt::ItemIsEditable);

    const EteraItem* eitem = witem->item();

    QString value = reinterpret_cast<QLineEdit*>(editor)->text();

    if(value == eitem->name())
        return;

    // базовые проверки имени
    QStringList chars;
    chars << "<"
          << ">"
          << ":"
          << "\""
          << "/"
          << "\\"
          << "|"
          << "?"
          << "*"
          << "\r"
          << "\n";

    for(auto&& ch: chars)
        if(value.contains(ch) == true) {
            witem->revertText();
            QMessageBox::warning(this, ATTENTION_MESSAGE, ERROR_MESSAGE_RENAME_INVALID_CHAR.arg(ch));
            return;
        }

    if(value == "." || value == "..") {
        witem->revertText();
        QMessageBox::warning(this, ATTENTION_MESSAGE, ERROR_MESSAGE_RENAME_IVALID_NAME);
        return;
    }

    QString path = m_path + value;

    EteraAPI* api = createAPI();

    ETERA_API_TASK_MV(api, task_on_rename_success, task_on_rename_error);

    m_tasks->addTask(api->id(), START_MESSAGE_RENAME.arg(eitem->path()).arg(path));

    api->mv(eitem->path(), path, false);
}
//----------------------------------------------------------------------------------------------

void WidgetDisk::task_on_rename_error(EteraAPI* api) {
    // откат переименования
    WidgetDiskItem* witem = findByPath(api->source());
    if(witem != nullptr)
        witem->revertText();

    if(api->lastErrorCode() == 409)
        QMessageBox::critical(this, ERROR_MESSAGE, ERROR_MESSAGE_RENAME_ALREADY_EXISTS.arg(api->source()).arg(api->target()));
    else
        QMessageBox::critical(this, ERROR_MESSAGE, ERROR_MESSAGE_RENAME.arg(api->source()).arg(api->target()).arg(api->lastErrorMessage()));

    releaseAPI(api);
}
//----------------------------------------------------------------------------------------------

void WidgetDisk::task_on_rename_success(EteraAPI* api) {
    EteraClipboard::instance()->removeByPath(api->source());

    ETERA_API_TASK_STAT(api, task_on_rename_stat_success, task_on_rename_stat_error);

    m_tasks->addTask(api->id(), START_MESSAGE_STAT.arg(api->target()));

    api->stat(api->target(), m_preview_arg, true);
}
//----------------------------------------------------------------------------------------------

void WidgetDisk::task_on_rename_stat_error(EteraAPI* api) {
    QMessageBox::critical(this, ERROR_MESSAGE, ERROR_MESSAGE_STAT.arg(api->target()).arg(api->lastErrorMessage()));

    releaseAPI(api);
}
//----------------------------------------------------------------------------------------------

void WidgetDisk::task_on_rename_stat_success(EteraAPI* api, const EteraItem& item) {
    WidgetDiskItem* witem = findByPath(api->source());
    if(witem != nullptr)
        witem->replaceItem(item, m_preview_mode);

    releaseAPI(api);
}
//----------------------------------------------------------------------------------------------

void WidgetDisk::menu_share_triggered() {
    shareObjects(true);
}
//----------------------------------------------------------------------------------------------

void WidgetDisk::menu_revoke_triggered() {
    shareObjects(false);
}
//----------------------------------------------------------------------------------------------

void WidgetDisk::shareObjects(bool share) {
    QList<QListWidgetItem*> selected = m_explorer->selectedItems();

    int count = selected.count();

    if(count == 0)
        return;

    quint64 parent = 0;
    if(count > 1) {
        parent = EteraAPI::nextID();
        if(share == true)
            m_tasks->addTask(parent, ROOT_MESSAGE_PUBLISH);
        else
            m_tasks->addTask(parent, ROOT_MESSAGE_UNPUBLISH);
    }

    for(int i = 0; i < count; ++i) {
        WidgetDiskItem* witem = static_cast<WidgetDiskItem*>(selected[i]);
        const EteraItem* eitem = witem->item();

        EteraAPI* api = createAPI();

        if(share == true && eitem->isPublic() == false) {
            ETERA_API_TASK_PUBLISH(api, task_on_publish_success, task_on_publish_error);

            m_tasks->addChildTask(parent, api->id(), START_MESSAGE_PUBLISH.arg(eitem->path()));

            api->publish(eitem->path());
        } else if(share == false && eitem->isPublic() == true) {
            ETERA_API_TASK_UNPUBLISH(api, task_on_unpublish_success, task_on_unpublish_error);

            m_tasks->addChildTask(parent, api->id(), START_MESSAGE_UNPUBLISH.arg(eitem->path()));

            api->unpublish(eitem->path());
        }
    }
}
//----------------------------------------------------------------------------------------------

void WidgetDisk::task_on_publish_error(EteraAPI* api) {
    if(api->lastErrorCode() == QNetworkReply::OperationCanceledError) {
        releaseAPI(api);
        return;
    }

    if(api->canRetry() == true) {
        delayTask(slot_task_on_publish_error, api);
        return;
    }

    if(messageBoxLocked() == true) {
        delayEmit(slot_task_on_publish_error, api);
        return;
    }

    messageBoxLock();
    QMessageBox::StandardButton reply = QMessageBox::critical(this, ERROR_MESSAGE, ERROR_MESSAGE_PUBLISH.arg(api->path()).arg(api->lastErrorMessage()), QMessageBox::Retry | QMessageBox::Ignore);
    messageBoxUnlock();

    if(api->deleted() == true)
        return;

    if(reply == QMessageBox::Retry)
        api->publish(api->path());
    else if(reply == QMessageBox::Ignore)
        releaseAPI(api);
}
//----------------------------------------------------------------------------------------------

void WidgetDisk::task_on_publish_success(EteraAPI* api) {
    WidgetDiskItem* witem = findByPath(api->path());

    if(witem == nullptr)
        releaseAPI(api);
    else {
        ETERA_API_TASK_STAT(api, task_on_publish_stat_success, task_on_publish_stat_error);

        m_tasks->addChildTask(api->parentID(), api->id(), START_MESSAGE_STAT.arg(api->path()));

        api->stat(api->path(), m_preview_arg, true);
    }
}
//----------------------------------------------------------------------------------------------

void WidgetDisk::task_on_unpublish_error(EteraAPI* api) {
    if(api->lastErrorCode() == QNetworkReply::OperationCanceledError) {
        releaseAPI(api);
        return;
    }

    if(api->canRetry() == true) {
        delayTask(slot_task_on_unpublish_error, api);
        return;
    }

    if(messageBoxLocked() == true) {
        delayEmit(slot_task_on_unpublish_error, api);
        return;
    }

    messageBoxLock();
    QMessageBox::StandardButton reply = QMessageBox::critical(this, ERROR_MESSAGE, ERROR_MESSAGE_UNPUBLISH.arg(api->path()).arg(api->lastErrorMessage()));
    messageBoxUnlock();

    if(api->deleted() == true)
        return;

    if(reply == QMessageBox::Retry)
        api->unpublish(api->path());
    else if(reply == QMessageBox::Ignore)
        releaseAPI(api);
}
//----------------------------------------------------------------------------------------------

void WidgetDisk::task_on_unpublish_success(EteraAPI* api) {
    WidgetDiskItem* witem = findByPath(api->path());

    if(witem == nullptr)
        releaseAPI(api);
    else {
        ETERA_API_TASK_STAT(api, task_on_unpublish_stat_success, task_on_unpublish_stat_error);

        m_tasks->addChildTask(api->parentID(), api->id(), START_MESSAGE_STAT.arg(api->path()));

        api->stat(api->path(), m_preview_arg, true);
    }
}
//----------------------------------------------------------------------------------------------

void WidgetDisk::task_on_publish_stat_error(EteraAPI* api) {
    if(api->lastErrorCode() == QNetworkReply::OperationCanceledError) {
        releaseAPI(api);
        return;
    }

    if(api->canRetry() == true) {
        delayTask(slot_task_on_publish_stat_error, api);
        return;
    }

    if(messageBoxLocked() == true) {
        delayEmit(slot_task_on_publish_stat_error, api);
        return;
    }

    messageBoxLock();
    QMessageBox::StandardButton reply = QMessageBox::critical(this, ERROR_MESSAGE, ERROR_MESSAGE_STAT.arg(api->path()).arg(api->lastErrorMessage()), QMessageBox::Retry | QMessageBox::Ignore);
    messageBoxUnlock();

    if(api->deleted() == true)
        return;

    if(reply == QMessageBox::Retry)
        api->stat(api->path(), m_preview_arg, true);
    else if(reply == QMessageBox::Ignore)
        releaseAPI(api);
}
//----------------------------------------------------------------------------------------------

void WidgetDisk::task_on_publish_stat_success(EteraAPI* api, const EteraItem& item) {
    WidgetDiskItem* witem = findByPath(item.path());

    if(witem != nullptr)
        witem->replaceItem(item, m_preview_mode);

    releaseAPI(api);
}
//----------------------------------------------------------------------------------------------

void WidgetDisk::task_on_unpublish_stat_error(EteraAPI* api) {
    if(api->lastErrorCode() == QNetworkReply::OperationCanceledError) {
        releaseAPI(api);
        return;
    }

    if(api->canRetry() == true) {
        delayTask(slot_task_on_unpublish_stat_error, api);
        return;
    }

    if(messageBoxLocked() == true) {
        delayEmit(slot_task_on_unpublish_stat_error, api);
        return;
    }

    messageBoxLock();
    QMessageBox::StandardButton reply = QMessageBox::critical(this, ERROR_MESSAGE, ERROR_MESSAGE_STAT.arg(api->path()).arg(api->lastErrorMessage()));
    messageBoxUnlock();

    if(api->deleted() == true)
        return;

    if(reply == QMessageBox::Retry)
        api->stat(api->path(), m_preview_arg, true);
    else if(reply == QMessageBox::Ignore)
        releaseAPI(api);
}
//----------------------------------------------------------------------------------------------

void WidgetDisk::task_on_unpublish_stat_success(EteraAPI* api, const EteraItem& item) {
    WidgetDiskItem* witem = findByPath(item.path());

    if(witem != nullptr)
        witem->replaceItem(item, m_preview_mode);

    releaseAPI(api);
}
//----------------------------------------------------------------------------------------------

void WidgetDisk::menu_info_triggered() {
    WidgetDiskItem* witem = static_cast<WidgetDiskItem*>(m_explorer->currentItem());
    if(witem == nullptr)
        return;

    const EteraItem* eitem = witem->item();

    FormInfo* info = new FormInfo(*eitem, this);
    info->show();
}
//----------------------------------------------------------------------------------------------

void WidgetDisk::dragEnterEvent(QDragEnterEvent* event) {
    if(event->mimeData()->hasFormat("text/uri-list") == true && m_path.isEmpty() == false)
        event->acceptProposedAction();
}
//----------------------------------------------------------------------------------------------

void WidgetDisk::dropEvent(QDropEvent* event) {
    if(m_path.isEmpty() == true)
        return;

    if(event->mimeData()->hasFormat("text/uri-list") == true) {
        QStringList list;

        QList<QUrl> urls = event->mimeData()->urls();
        for(auto&& url: urls)
            if(url.scheme() == "file")
                list.append(url.toLocalFile());

        if(list.isEmpty() == false)
            putLocalObjects(list);

        event->acceptProposedAction();
    }
}
//----------------------------------------------------------------------------------------------

void WidgetDisk::putLocalObjects(const QStringList& paths) {
    if(m_path.isEmpty() == true || paths.isEmpty() == true)
        return;

    quint64 parent = EteraAPI::nextID();
    m_tasks->addTask(parent, ROOT_MESSAGE_UPLOAD);

    for(auto&& path: paths)
        putLocalObject(path, parent);

    m_tasks->checkTask(parent);
}
//----------------------------------------------------------------------------------------------

void WidgetDisk::putLocalObject(const QString& path, quint64 parent) {
    if(m_path.isEmpty() == true)
        return;

    QFileInfo info(path);

    // пропускаем симлинки и bundle
    if(info.isDir() == false && info.isFile() == false)
        return;

    QString target = m_path + info.fileName();

    if(info.isDir() == true)
        putLocalDir(path, target, false, parent);
    else if(info.isFile() == true)
        putLocalFile(path, target, false, parent);
}
//----------------------------------------------------------------------------------------------

void WidgetDisk::putLocalDir(const QString& source, const QString& target, bool overwrite, quint64 parent) {
    addPutActivity(epatDir, parent, source, target, overwrite);
}
//----------------------------------------------------------------------------------------------

void WidgetDisk::task_on_put_mkdir_error(EteraAPI* api) {
    if(api->lastErrorCode() == QNetworkReply::OperationCanceledError) {
        releaseAPI(api);
        return;
    }

    if(api->canRetry() == true) {
        delayTask(slot_task_on_put_mkdir_error, api);
        return;
    }

    // если объект существует, нужно убедиться, что это директория и тогда можно продолжить работу
    if(api->lastErrorCode() == 409) {
        ETERA_API_TASK_STAT(api, task_on_put_ensure_success, task_on_put_ensure_error);

        m_tasks->addChildTask(api->parentID(), api->id(), START_MESSAGE_STAT.arg(api->path()));

        api->stat(api->path());

        return;
    }

    if(messageBoxLocked() == true) {
        delayEmit(slot_task_on_put_mkdir_error, api);
        return;
    }

    messageBoxLock();
    QMessageBox::StandardButton reply = QMessageBox::critical(this, ERROR_MESSAGE, ERROR_MESSAGE_MKDIR.arg(api->path()).arg(api->lastErrorMessage()), QMessageBox::Retry | QMessageBox::Abort | QMessageBox::Ignore);
    messageBoxUnlock();

    if(api->deleted() == true)
        return;

    if(reply == QMessageBox::Retry)
        api->mkdir(api->path());
    else if(reply == QMessageBox::Abort)
        abortPutActivity(api->id(), true);
    else if(reply == QMessageBox::Ignore)
        spawnPutActivity(epatDir, api);
}
//----------------------------------------------------------------------------------------------

void WidgetDisk::task_on_put_mkdir_success(EteraAPI* api) {
    if(api->deleted() == true)
        return;

    { // TODO: блок нужен только если директория создана в текущей области видимости
        ETERA_API_TASK_STAT(api, task_on_put_stat_success, task_on_put_stat_error);

        m_tasks->addChildTask(api->parentID(), api->id(), START_MESSAGE_STAT.arg(api->path()));

        api->stat(api->path());
    }

    syncLocalDir(api->source(), api->target(), api->overwrite(), api->parentID());

    // TODO: если блок выше будет по условию
    // spawnPutActivity(epatDir, api);
}
//----------------------------------------------------------------------------------------------

void WidgetDisk::syncLocalDir(const QString& source, const QString& target, bool overwrite, quint64 parent) {
    QFileInfoList list = QDir(source).entryInfoList(QDir::AllEntries | QDir::Hidden | QDir::System | QDir::NoDotAndDotDot);

    for(QFileInfo info: list) {
        if(info.isDir() == true)
            putLocalDir(info.absoluteFilePath(), target + "/" + info.fileName(), overwrite, parent);
        else if(info.isFile() == true)
            putLocalFile(info.absoluteFilePath(), target + "/" + info.fileName(), overwrite, parent);
    }
}
//----------------------------------------------------------------------------------------------

void WidgetDisk::putLocalFile(const QString& source, const QString& target, bool overwrite, quint64 parent) {
    addPutActivity(epatFile, parent, source, target, overwrite);
}
//----------------------------------------------------------------------------------------------

void WidgetDisk::task_on_put_file_error(EteraAPI* api) {
    if(api->lastErrorCode() == QNetworkReply::OperationCanceledError) {
        releaseAPI(api);
        return;
    }

    if(api->canRetry() == true) {
        m_tasks->resetProgress(api->id());
        delayTask(slot_task_on_put_file_error, api);
        return;
    }

    if(messageBoxLocked() == true) {
        delayEmit(slot_task_on_put_file_error, api);
        return;
    }

    // если объект существует, нужно убедиться, что это файл и тогда можно продолжить работу
    if(api->lastErrorCode() == 409) {
        quint64 rootid = m_tasks->rootID(api->id());
        QMessageBox::StandardButton reply = m_tasks->reply(rootid);

        // пропускаем если ранее было указано не перезаписывать файлы
        if(reply != QMessageBox::NoToAll) {
            ETERA_API_TASK_STAT(api, task_on_put_ensure_success, task_on_put_ensure_error);

            m_tasks->addChildTask(api->parentID(), api->id(), START_MESSAGE_STAT.arg(api->target()));

            api->stat(api->target());
        } else
            spawnPutActivity(epatFile, api);

        return;
    }

    messageBoxLock();
    QMessageBox::StandardButton reply = QMessageBox::critical(this, ERROR_MESSAGE, ERROR_MESSAGE_UPLOAD.arg(api->source()).arg(api->target()).arg(api->lastErrorMessage()), QMessageBox::Retry | QMessageBox::Abort | QMessageBox::Ignore);
    messageBoxUnlock();

    if(api->deleted() == true)
        return;

    if(reply == QMessageBox::Retry)
        api->put(api->source(), api->target(), api->overwrite());
    else if(reply == QMessageBox::Abort)
        abortPutActivity(api->id(), true);
    else if(reply == QMessageBox::Ignore)
        spawnPutActivity(epatFile, api);
}
//----------------------------------------------------------------------------------------------

void WidgetDisk::task_on_put_file_success(EteraAPI* api) {
    api->device()->deleteLater();

    if(api->deleted() == true)
        return;

    { // TODO: блок нужен только если файл создан в текущей области видимости
        ETERA_API_TASK_STAT(api, task_on_put_stat_success, task_on_put_stat_error);

        m_tasks->addChildTask(api->parentID(), api->id(), START_MESSAGE_STAT.arg(api->target()));

        api->stat(api->target(), m_preview_arg, true);
    }

    // TODO: если блок выше будет по условию
    // spawnPutActivity(epatFile, api);
}
//----------------------------------------------------------------------------------------------

void WidgetDisk::task_on_put_file_progress(EteraAPI* api, qint64 done, qint64 total) {
    if(api->deleted() == true)
        return;

    m_tasks->setProgress(api->id(), done, total);
}
//----------------------------------------------------------------------------------------------

void WidgetDisk::task_on_put_stat_error(EteraAPI* api) {
    if(api->lastErrorCode() == QNetworkReply::OperationCanceledError) {
        releaseAPI(api);
        return;
    }

    if(api->canRetry() == true) {
        delayTask(slot_task_on_put_stat_error, api);
        return;
    }

    if(messageBoxLocked() == true) {
        delayEmit(slot_task_on_put_stat_error, api);
        return;
    }

    messageBoxLock();
    QMessageBox::StandardButton reply = QMessageBox::critical(this, ERROR_MESSAGE, ERROR_MESSAGE_STAT.arg(api->path()).arg(api->lastErrorMessage()), QMessageBox::Retry | QMessageBox::Abort | QMessageBox::Ignore);
    messageBoxUnlock();

    if(api->deleted() == true)
        return;

    if(reply == QMessageBox::Retry)
        api->stat(api->path());
    else if(reply == QMessageBox::Abort)
        abortPutActivity(api->id(), true);
    else if(reply == QMessageBox::Ignore) {
        if(api->ensure() == eitFile)
            spawnPutActivity(epatFile, api);
        else if(api->ensure() == eitDir)
            spawnPutActivity(epatDir, api);
    }
}
//----------------------------------------------------------------------------------------------

void WidgetDisk::task_on_put_stat_success(EteraAPI* api, const EteraItem& item) {
    if(api->deleted() == true)
        return;

    if(item.parentPath() == m_path) {
        WidgetDiskItem* witem = findByPath(item.path());

        if(witem == nullptr)
            m_explorer->setCurrentItem(new WidgetDiskItem(m_explorer, item, m_preview_mode), QItemSelectionModel::ClearAndSelect);
        else
            witem->replaceItem(item, m_preview_mode);
    }

    if(api->ensure() == eitFile)
        spawnPutActivity(epatFile, api);
    else if(api->ensure() == eitDir)
        spawnPutActivity(epatDir, api);
}
//----------------------------------------------------------------------------------------------

void WidgetDisk::task_on_put_ensure_error(EteraAPI* api) {
    if(api->lastErrorCode() == QNetworkReply::OperationCanceledError) {
        releaseAPI(api);
        return;
    }

    if(api->canRetry() == true) {
        delayTask(slot_task_on_put_ensure_error, api);
        return;
    }

    if(messageBoxLocked() == true) {
        delayEmit(slot_task_on_put_ensure_error, api);
        return;
    }

    messageBoxLock();
    QMessageBox::StandardButton reply = QMessageBox::critical(this, ERROR_MESSAGE, ERROR_MESSAGE_STAT.arg(api->path()).arg(api->lastErrorMessage()), QMessageBox::Retry | QMessageBox::Abort | QMessageBox::Ignore);
    messageBoxUnlock();

    if(api->deleted() == true)
        return;

    if(reply == QMessageBox::Retry)
        api->stat(api->path());
    else if(reply == QMessageBox::Abort)
        abortPutActivity(api->id(), true);
    else if(reply == QMessageBox::Ignore) {
        if(api->ensure() == eitFile)
            spawnPutActivity(epatFile, api);
        else if(api->ensure() == eitDir)
            spawnPutActivity(epatDir, api);
    }
}
//----------------------------------------------------------------------------------------------

void WidgetDisk::task_on_put_ensure_success(EteraAPI* api, const EteraItem& item) {
    if(api->ensure() == eitDir) {
        if(item.isDir() == true)
            syncLocalDir(api->source(), api->target(), api->overwrite(), api->parentID());
        else if(item.isFile() == true) {
            quint64 rootid = m_tasks->rootID(api->id());
            QMessageBox::StandardButton reply = m_tasks->reply(rootid);

            // пропускаем если ранее было указано не перезаписывать файлы
            if(reply != QMessageBox::NoToAll) {
                // что делать с конфликтами?
                if(reply != QMessageBox::YesToAll) {
                    if(messageBoxLocked() == true) {
                        delayEmit(slot_task_on_put_ensure_success, api, item);
                        return;
                    }

                    messageBoxLock();
                    reply = QMessageBox::question(this, START_MESSAGE_UPLOAD_CAPTION, START_MESSAGE_UPLOAD_TEXT.arg(api->path()), QMessageBox::Yes | QMessageBox::No | QMessageBox::YesToAll | QMessageBox::NoToAll);
                    messageBoxUnlock();

                    if(api->deleted() == true)
                        return;

                    m_tasks->setReply(rootid, reply);
                }

                // удалить файл и перезаписать директорией при положительном ответе
                if(reply == QMessageBox::Yes || reply == QMessageBox::YesToAll) {
                    bool overwrite = (api->overwrite() == true || reply == QMessageBox::YesToAll);

                    api->setOverwrite(overwrite);

                    ETERA_API_TASK_RM(api, task_on_put_rm_success, task_on_put_rm_error);

                    m_tasks->addChildTask(api->parentID(), api->id(), START_MESSAGE_RM.arg(api->path()));

                    api->rm(api->path(), true);

                    return;
                }
            }
        }
    } else if(api->ensure() == eitFile) {
        quint64 rootid = m_tasks->rootID(api->id());
        QMessageBox::StandardButton reply = m_tasks->reply(rootid);

        // пропускаем если ранее было указано не перезаписывать файлы
        if(reply != QMessageBox::NoToAll) {
            // что делать с конфликтами?
            if(reply != QMessageBox::YesToAll) {
                if(messageBoxLocked() == true) {
                    delayEmit(slot_task_on_put_ensure_success, api, item);
                    return;
                }

                messageBoxLock();
                reply = QMessageBox::question(this, START_MESSAGE_UPLOAD_CAPTION, START_MESSAGE_UPLOAD_TEXT.arg(api->path()), QMessageBox::Yes | QMessageBox::No | QMessageBox::YesToAll | QMessageBox::NoToAll);
                messageBoxUnlock();

                if(api->deleted() == true)
                    return;

                m_tasks->setReply(rootid, reply);
            }

            // удалить и перезаписать файлом при положительном ответе
            if(reply == QMessageBox::Yes || reply == QMessageBox::YesToAll) {
                if(item.isFile() == true) {
                    ETERA_API_TASK_PUT(api, task_on_put_file_success, task_on_put_file_error, task_on_put_file_progress);

                    m_tasks->addChildTask(api->parentID(), api->id(), localBasename(api->source()), START_MESSAGE_UPLOAD.arg(api->source()).arg(api->target()));

                    api->put(api->source(), api->target(), true);
                } else if(item.isDir() == true) {
                    bool overwrite = (api->overwrite() == true || reply == QMessageBox::YesToAll);

                    api->setOverwrite(overwrite);

                    ETERA_API_TASK_RM(api, task_on_put_rm_success, task_on_put_rm_error);

                    m_tasks->addChildTask(api->parentID(), api->id(), START_MESSAGE_RM.arg(api->path()));

                    api->rm(api->path(), true);
                }

                return;
            }
        }
    }

    if(api->ensure() == eitFile)
        spawnPutActivity(epatFile, api);
    else if(api->ensure() == eitDir)
        spawnPutActivity(epatDir, api);
}
//----------------------------------------------------------------------------------------------

void WidgetDisk::task_on_put_rm_error(EteraAPI* api) {
    if(api->lastErrorCode() == QNetworkReply::OperationCanceledError) {
        releaseAPI(api);
        return;
    }

    if(api->canRetry() == true) {
        delayTask(slot_task_on_put_rm_error, api);
        return;
    }

    if(messageBoxLocked() == true) {
        delayEmit(slot_task_on_put_rm_error, api);
        return;
    }

    messageBoxLock();
    QMessageBox::StandardButton reply = QMessageBox::critical(this, ERROR_MESSAGE, ERROR_MESSAGE_RM.arg(api->path()).arg(api->lastErrorMessage()), QMessageBox::Retry | QMessageBox::Abort | QMessageBox::Ignore);
    messageBoxUnlock();

    if(api->deleted() == true)
        return;

    if(reply == QMessageBox::Retry)
        api->rm(api->path(), true);
    else if(reply == QMessageBox::Abort)
        abortPutActivity(api->id(), true);
    else if(reply == QMessageBox::Ignore) {
        if(api->ensure() == eitFile)
            spawnPutActivity(epatFile, api);
        else if(api->ensure() == eitDir)
            spawnPutActivity(epatDir, api);
    }
}
//----------------------------------------------------------------------------------------------

void WidgetDisk::task_on_put_rm_success(EteraAPI* api) {
    if(api->deleted() == true)
        return;

    if(api->ensure() == eitDir) {
        ETERA_API_TASK_MKDIR(api, task_on_put_mkdir_success, task_on_put_mkdir_error);

        m_tasks->addChildTask(api->parentID(), api->id(), START_MESSAGE_MKDIR.arg(api->path()));

        api->mkdir(api->path());
    } else if(api->ensure() == eitFile) {
        ETERA_API_TASK_PUT(api, task_on_put_file_success, task_on_put_file_error, task_on_put_file_progress);

        m_tasks->addChildTask(api->parentID(), api->id(), localBasename(api->source()), START_MESSAGE_UPLOAD.arg(api->source()).arg(api->target()));

        api->put(api->source(), api->target(), api->overwrite());
    }
}
//----------------------------------------------------------------------------------------------

void WidgetDisk::addPutActivity(EteraPutActivityType type, quint64 parent, const QString& source, const QString& target, bool overwrite) {
    if(type != epatUnknown) {
        EteraPutActivityItem item;

        item.ID = EteraAPI::nextID();
        item.Parent = parent;
        item.Source = source;
        item.Target = target;
        item.Overwrite = overwrite;

        if(type == epatDir) {
            m_put_queue_mkdir.enqueue(item);
            m_tasks->addChildTask(item.Parent, item.ID, START_MESSAGE_MKDIR.arg(item.Source));
        } else if(type == epatFile) {
            m_put_queue_put.enqueue(item);
            m_tasks->addChildTask(item.Parent, item.ID, localBasename(item.Source), START_MESSAGE_UPLOAD.arg(item.Source).arg(item.Target));
        }
    }

    spawnPutActivity(epatUnknown);
}
//----------------------------------------------------------------------------------------------

void WidgetDisk::spawnPutActivity(EteraPutActivityType type, EteraAPI* api) {
    Q_UNUSED(type);

    if(api != nullptr) {
        m_put_active_api_mkdir.remove(api->id());
        m_put_active_api_put.remove(api->id());
        releaseAPI(api);
    }

    while(m_put_active_api_mkdir.count() + m_put_active_api_put.count() < m_put_activity_limit && m_put_queue_put.empty() == false) {
        EteraPutActivityItem item = m_put_queue_put.dequeue();

        api = createAPI(item.ID);

        api->setParentID(item.Parent);
        api->setEnsure(eitFile);

        ETERA_API_TASK_PUT(api, task_on_put_file_success, task_on_put_file_error, task_on_put_file_progress);

        m_put_active_api_put[item.ID] = api;

        api->put(item.Source, item.Target, item.Overwrite);
    }

    // mkdir активностей должно быть не более одной - более логично (последовательно) обходит директории при записи
    if(m_put_active_api_mkdir.count() == 0 && m_put_active_api_put.count() < m_put_activity_limit && m_put_queue_mkdir.empty() == false) {
        EteraPutActivityItem item = m_put_queue_mkdir.dequeue();

        api = createAPI(item.ID);

        api->setSource(item.Source);
        api->setTarget(item.Target);
        api->setParentID(item.Parent);
        api->setOverwrite(item.Overwrite);
        api->setEnsure(eitDir);

        ETERA_API_TASK_MKDIR(api, task_on_put_mkdir_success, task_on_put_mkdir_error);

        m_put_active_api_mkdir[item.ID] = api;

        api->mkdir(item.Target);
    }
}
//----------------------------------------------------------------------------------------------

void WidgetDisk::abortPutActivity(quint64 id, bool full) {
    if(full == true) {
        id = m_tasks->rootID(id);
        if(id == 0)
            return;
    }

    QList<quint64> aborted;
    m_tasks->childIDs(id, aborted);

    removePutActivity(m_put_queue_mkdir, aborted);
    removePutActivity(m_put_queue_put, aborted);

    for(quint64 id: aborted) {
        EteraAPI* api = m_put_active_api_mkdir.value(id, nullptr);
        if(api != nullptr) {
            removeDelayed(api);
            m_put_active_api_mkdir.remove(api->id());
            api->abort();
            releaseAPI(api);
        }

        api = m_put_active_api_put.value(id, nullptr);
        if(api != nullptr) {
            removeDelayed(api);
            m_put_active_api_put.remove(api->id());
            api->abort();
            releaseAPI(api);
        }
    }
}
//----------------------------------------------------------------------------------------------

void WidgetDisk::removePutActivity(EteraPutActivityQueue& queue, QList<quint64>& aborted) {
    EteraPutActivityQueue::iterator i = queue.begin();
    while(i != queue.end()) {
        EteraPutActivityItem item = *i;
        int index = aborted.indexOf(item.ID);
        if(index != -1) {
            i = queue.erase(i);
            aborted.removeAt(index);
            m_tasks->removeTask(item.ID);
        } else
            ++i;
    }
}
//----------------------------------------------------------------------------------------------

void WidgetDisk::getRemoteObjects(const QString& path) {
    QList<QListWidgetItem*> selected = m_explorer->selectedItems();

    int count = selected.count();

    if(count == 0)
        return;

    quint64 parent = EteraAPI::nextID();
    m_tasks->addTask(parent, ROOT_MESSAGE_DOWNLOAD);

    for(int i = 0; i < count; ++i) {
        WidgetDiskItem* witem = static_cast<WidgetDiskItem*>(selected[i]);
        const EteraItem* eitem = witem->item();

        QMessageBox::StandardButton reply = QMessageBox::Ok;
        if(eitem->isDir() == true)
            reply = getRemoteDir(eitem->path(), path + "/" + eitem->name(), parent);
        else if(eitem->isFile() == true)
            reply = getRemoteFile(eitem->path(), path + "/" + eitem->name(), parent);

        Q_ASSERT(reply == QMessageBox::NoButton || reply == QMessageBox::Abort);

        if(reply == QMessageBox::NoButton)
            continue;
        else if(reply == QMessageBox::Abort) {
            abortGetActivity(parent, true);
            break;
        }
    }

    m_tasks->checkTask(parent);
}
//----------------------------------------------------------------------------------------------

QMessageBox::StandardButton WidgetDisk::getRemoteDir(const QString& source, const QString& target, quint64 parent) {
    QFileInfo info(target);

    if(info.exists() == false) {
        QMessageBox::StandardButton reply = QMessageBox::Retry;
        while(reply == QMessageBox::Retry && info.dir().mkdir(info.absoluteFilePath()) == false)
            reply = QMessageBox::critical(this, ERROR_MESSAGE, ERROR_MESSAGE_MKDIR.arg(info.absoluteFilePath()).arg(ERROR_MESSAGE_QT), QMessageBox::Retry | QMessageBox::Abort);

        if(reply != QMessageBox::Retry)
            return reply;
    } else if(info.exists() == true && info.isDir() == false) {
        quint64 rootid = m_tasks->rootID(parent);
        QMessageBox::StandardButton reply = m_tasks->reply(rootid);

        // ранее было указано не перезаписывать файлы
        if(reply == QMessageBox::NoToAll)
            return QMessageBox::NoButton;

        // что делать с конфликтами?
        if(reply != QMessageBox::YesToAll) {
            reply = QMessageBox::question(this, START_MESSAGE_DOWNLOAD_CAPTION, START_MESSAGE_DOWNLOAD_TEXT.arg(target), QMessageBox::Yes | QMessageBox::No | QMessageBox::YesToAll | QMessageBox::NoToAll);

            m_tasks->setReply(rootid, reply);

            // оставить локальную копию
            if(reply == QMessageBox::No || reply == QMessageBox::NoToAll)
                return QMessageBox::NoButton;
        }

        // QMessageBox::YesToAll
        reply = QMessageBox::Retry;
        while(reply == QMessageBox::Retry && info.dir().remove(info.absoluteFilePath()) == false)
            reply = QMessageBox::critical(this, ERROR_MESSAGE, ERROR_MESSAGE_RM.arg(info.absoluteFilePath()).arg(ERROR_MESSAGE_QT), QMessageBox::Retry | QMessageBox::Abort);

        if(reply != QMessageBox::Retry)
            return reply;
    }

    addGetActivity(egatList, parent, source, target);

    return QMessageBox::NoButton;
}
//----------------------------------------------------------------------------------------------

QMessageBox::StandardButton WidgetDisk::getRemoteFile(const QString& source, const QString& target, quint64 parent) {
    QFileInfo info(target);

    if(info.exists() == true) {
        quint64 rootid = m_tasks->rootID(parent);
        QMessageBox::StandardButton reply = m_tasks->reply(rootid);

        // ранее было указано не перезаписывать файлы
        if(reply == QMessageBox::NoToAll)
            return QMessageBox::NoButton;

        // что делать с конфликтами?
        if(reply != QMessageBox::YesToAll) {
            reply = QMessageBox::question(this, START_MESSAGE_DOWNLOAD_CAPTION, START_MESSAGE_DOWNLOAD_TEXT.arg(target), QMessageBox::Yes | QMessageBox::No | QMessageBox::YesToAll | QMessageBox::NoToAll);

            m_tasks->setReply(rootid, reply);

            // оставить локальную копию
            if(reply == QMessageBox::No || reply == QMessageBox::NoToAll)
                return QMessageBox::NoButton;
        }

        // QMessageBox::YesToAll
        if(info.isDir() == true) {
            QMessageBox::StandardButton reply = removeDir(target);
            if(reply != QMessageBox::NoButton)
                return reply;
        } else if(info.isFile() == true || info.isSymLink() == true) {
            QMessageBox::StandardButton reply = QMessageBox::Retry;
            while(reply == QMessageBox::Retry && info.dir().remove(info.absoluteFilePath()) == false)
                reply = QMessageBox::critical(this, ERROR_MESSAGE, ERROR_MESSAGE_RM.arg(info.absoluteFilePath()).arg(ERROR_MESSAGE_QT), QMessageBox::Retry | QMessageBox::Abort);

            if(reply != QMessageBox::Retry)
                return reply;
        } else
            // никогда не должно случиться
            qFatal("Unknown object (not dir, not file)");
    }

    addGetActivity(agatGet, parent, source, target);

    return QMessageBox::NoButton;
}
//----------------------------------------------------------------------------------------------

void WidgetDisk::task_on_get_file_error(EteraAPI* api) {
    if(api->lastErrorCode() == QNetworkReply::OperationCanceledError) {
        releaseAPI(api);
        return;
    }

    if(api->canRetry() == true) {
        m_tasks->resetProgress(api->id());
        delayTask(slot_task_on_get_file_error, api);
        return;
    }

    if(messageBoxLocked() == true) {
        delayEmit(slot_task_on_get_file_error, api);
        return;
    }

    messageBoxLock();
    QMessageBox::StandardButton reply = QMessageBox::critical(this, ERROR_MESSAGE, ERROR_MESSAGE_DOWNLOAD.arg(api->source()).arg(api->target()).arg(api->lastErrorMessage()), QMessageBox::Retry | QMessageBox::Abort | QMessageBox::Ignore);
    messageBoxUnlock();

    if(api->deleted() == true)
        return;

    if(reply == QMessageBox::Retry)
        api->get(api->source(), api->target());
    else if(reply == QMessageBox::Abort)
        abortGetActivity(api->id(), true);
    else if(reply == QMessageBox::Ignore)
        spawnGetActivity(agatGet, api);
}
//----------------------------------------------------------------------------------------------

void WidgetDisk::task_on_get_file_success(EteraAPI* api) {
    if(api->deleted() == true)
        return;

    spawnGetActivity(agatGet, api);
}
//----------------------------------------------------------------------------------------------

void WidgetDisk::task_on_get_file_progress(EteraAPI* api, qint64 done, qint64 total) {
    if(api->deleted() == true)
        return;

    m_tasks->setProgress(api->id(), done, total);
}
//----------------------------------------------------------------------------------------------

QMessageBox::StandardButton WidgetDisk::removeDir(QDir dir) {
    if(dir.exists() == false)
        return QMessageBox::NoButton;

    QFileInfoList list = dir.entryInfoList(QDir::AllEntries | QDir::Hidden | QDir::System | QDir::NoDotAndDotDot);

    for(auto&& info: list) {
        if(info.isFile() == true || info.isSymLink() == true) {
            QMessageBox::StandardButton reply = QMessageBox::Retry;
            while(reply == QMessageBox::Retry && dir.remove(info.absoluteFilePath()) == false)
                reply = QMessageBox::critical(this, ERROR_MESSAGE, ERROR_MESSAGE_RM.arg(info.absoluteFilePath()).arg(ERROR_MESSAGE_QT), QMessageBox::Retry | QMessageBox::Abort);
        } else if(info.isDir() == true) {
            QMessageBox::StandardButton reply = removeDir(info.absoluteFilePath());
            if(reply != QMessageBox::NoButton)
                return reply;
        }
    }

    QMessageBox::StandardButton reply = QMessageBox::Retry;
    while(reply == QMessageBox::Retry && dir.rmdir(dir.absolutePath()) == false)
        reply = QMessageBox::critical(this, ERROR_MESSAGE, ERROR_MESSAGE_RM.arg(dir.absolutePath()).arg(ERROR_MESSAGE_QT), QMessageBox::Retry | QMessageBox::Abort);

    return QMessageBox::NoButton;
}
//----------------------------------------------------------------------------------------------

void WidgetDisk::task_on_get_dir_error(EteraAPI* api) {
    if(api->lastErrorCode() == QNetworkReply::OperationCanceledError) {
        releaseAPI(api);
        return;
    }

    if(api->canRetry() == true) {
        delayTask(slot_task_on_get_dir_error, api);
        return;
    }

    if(messageBoxLocked() == true) {
        delayEmit(slot_task_on_get_dir_error, api);
        return;
    }

    messageBoxLock();
    QMessageBox::StandardButton reply = QMessageBox::critical(this, ERROR_MESSAGE, ERROR_MESSAGE_LS.arg(api->path()).arg(api->lastErrorMessage()), QMessageBox::Retry | QMessageBox::Abort | QMessageBox::Ignore);
    messageBoxUnlock();

    if(api->deleted() == true)
        return;

    if(reply == QMessageBox::Retry)
        api->ls(api->path(), api->preview(), api->crop(), api->offset(), api->limit());
    else if(reply == QMessageBox::Abort)
        abortGetActivity(api->id(), true);
    else if(reply == QMessageBox::Ignore)
        spawnGetActivity(egatList, api);
}
//----------------------------------------------------------------------------------------------

void WidgetDisk::task_on_get_dir_success(EteraAPI* api, const EteraItemList& list, quint64 limit) {
    if(api->deleted() == true)
        return;

    for(auto&& item: list) {
        QMessageBox::StandardButton reply = QMessageBox::Ok;
        if(item.isDir() == true)
            reply = getRemoteDir(item.path(), api->target() + "/" + item.name(), api->id());
        else if(item.isFile() == true)
            reply = getRemoteFile(item.path(), api->target() + "/" + item.name(), api->id());

        Q_ASSERT(reply == QMessageBox::NoButton || reply == QMessageBox::Abort);

        if(reply == QMessageBox::NoButton)
            continue;
        else if(reply == QMessageBox::Abort) {
            abortGetActivity(api->id(), true);
            return;
        }
    }

    if((quint64)list.count() < limit)
        spawnGetActivity(egatList, api);
    else {
        quint64 offset = api->offset() + limit;
        api->ls(api->path(), api->preview(), api->crop(), offset, limit);
    }
}
//----------------------------------------------------------------------------------------------

void WidgetDisk::addGetActivity(EteraGetActivityType type, quint64 parent, const QString& source, const QString& target) {
    if(type != agatUnknown) {
        EteraGetActivityItem item;

        item.ID = EteraAPI::nextID();
        item.Parent = parent;
        item.Source = source;
        item.Target = target;

        if(type == egatList) {
            m_get_queue_ls.enqueue(item);
            m_tasks->addChildTask(item.Parent, item.ID, START_MESSAGE_LS.arg(item.Source));
        } else if(type == agatGet) {
            m_get_queue_get.enqueue(item);
            m_tasks->addChildTask(item.Parent, item.ID, remoteBasename(item.Source), START_MESSAGE_DOWNLOAD.arg(item.Source).arg(item.Target));
        }
    }

    spawnGetActivity(agatUnknown);
}
//----------------------------------------------------------------------------------------------

void WidgetDisk::spawnGetActivity(EteraGetActivityType type, EteraAPI* api) {
    Q_UNUSED(type);

    if(api != nullptr) {
        m_get_active_api_ls.remove(api->id());
        m_get_active_api_get.remove(api->id());
        releaseAPI(api);
    }

    while(m_get_active_api_ls.count() + m_get_active_api_get.count() < m_get_activity_limit && m_get_queue_get.empty() == false) {
        EteraGetActivityItem item = m_get_queue_get.dequeue();

        api = createAPI(item.ID);

        api->setParentID(item.Parent);

        ETERA_API_TASK_GET(api, task_on_get_file_success, task_on_get_file_error, task_on_get_file_progress);

        m_get_active_api_get[item.ID] = api;

        api->get(item.Source, item.Target);
    }

    // ls активностей должно быть не более одной - это дает возможность не показывать более одного диалога
    // о перезаписи файла плюс более логично (последовательно) обходит директории при скачивании
    if(m_get_active_api_ls.count() == 0 && m_get_active_api_get.count() < m_get_activity_limit && m_get_queue_ls.empty() == false) {
        EteraGetActivityItem item = m_get_queue_ls.dequeue();

        api = createAPI(item.ID);

        api->setSource(item.Source);
        api->setTarget(item.Target);
        api->setParentID(item.Parent);

        ETERA_API_TASK_LS(api, task_on_get_dir_success, task_on_get_dir_error);

        m_get_active_api_ls[item.ID] = api;

        api->ls(item.Source);
    }
}
//----------------------------------------------------------------------------------------------

void WidgetDisk::abortGetActivity(quint64 id, bool full) {
    if(full == true) {
        id = m_tasks->rootID(id);
        if(id == 0)
            return;
    }

    QList<quint64> aborted;
    m_tasks->childIDs(id, aborted);

    removeGetActivity(m_get_queue_ls, aborted);
    removeGetActivity(m_get_queue_get, aborted);

    for(quint64 id: aborted) {
        EteraAPI* api = m_get_active_api_ls.value(id, nullptr);
        if(api != nullptr) {
            removeDelayed(api);
            m_get_active_api_ls.remove(api->id());
            api->abort();
            releaseAPI(api);
        }

        api = m_get_active_api_get.value(id, nullptr);
        if(api != nullptr) {
            removeDelayed(api);
            m_get_active_api_get.remove(api->id());
            api->abort();
            releaseAPI(api);
        }
    }
}
//----------------------------------------------------------------------------------------------

void WidgetDisk::removeGetActivity(EteraGetActivityQueue& queue, QList<quint64>& aborted) {
    EteraGetActivityQueue::iterator i = queue.begin();
    while(i != queue.end()) {
        EteraGetActivityItem item = *i;
        int index = aborted.indexOf(item.ID);
        if(index != -1) {
            i = queue.erase(i);
            aborted.removeAt(index);
            m_tasks->removeTask(item.ID);
        } else
            ++i;
    }
}
//----------------------------------------------------------------------------------------------

void WidgetDisk::delayEmit(EteraTaskSlot slot, EteraAPI* api) {
    EteraTaskSignal signal;

    signal.Slot = slot;
    signal.API = api;

    m_delayed_queue.enqueue(signal);
}
//----------------------------------------------------------------------------------------------

void WidgetDisk::delayEmit(EteraTaskSlotStat slot, EteraAPI* api, const EteraItem& item) {
    EteraTaskSignalStat signal;

    signal.Slot = slot;
    signal.API = api;
    signal.Item = item;

    m_delayed_stat_queue.enqueue(signal);
}
//----------------------------------------------------------------------------------------------

void WidgetDisk::delayTask(EteraTaskSlot slot, EteraAPI* api) {
    EteraTaskSignal signal;

    signal.Slot = slot;
    signal.API = api;

    m_delayed_task_queue.enqueue(signal);
}
//----------------------------------------------------------------------------------------------

void WidgetDisk::emit_delayed_signals() {
    while(m_message_box_active == false && m_delayed_queue.count() > 0) {
        EteraTaskSignal signal = m_delayed_queue.dequeue();
        switch(signal.Slot) {
        case slot_task_on_ls_error: task_on_ls_error(signal.API); break;
        case slot_task_on_mkdir_error: task_on_mkdir_error(signal.API); break;
        case slot_task_on_mkdir_stat_error: task_on_mkdir_stat_error(signal.API); break;
        case slot_task_on_copy_paste_error: task_on_copy_paste_error(signal.API); break;
        case slot_task_on_cut_paste_error: task_on_cut_paste_error(signal.API); break;
        case slot_task_on_copy_cut_paste_stat_error: task_on_copy_cut_paste_stat_error(signal.API); break;
        case slot_task_on_rm_error: task_on_rm_error(signal.API); break;
        case slot_task_on_publish_error: task_on_publish_error(signal.API); break;
        case slot_task_on_unpublish_error: task_on_unpublish_error(signal.API); break;
        case slot_task_on_publish_stat_error: task_on_publish_stat_error(signal.API); break;
        case slot_task_on_unpublish_stat_error: task_on_unpublish_stat_error(signal.API); break;
        case slot_task_on_put_mkdir_error: task_on_put_mkdir_error(signal.API); break;
        case slot_task_on_put_file_error: task_on_put_file_error(signal.API); break;
        case slot_task_on_put_stat_error: task_on_put_stat_error(signal.API); break;
        case slot_task_on_put_ensure_error: task_on_put_ensure_error(signal.API); break;
        case slot_task_on_put_rm_error: task_on_put_rm_error(signal.API); break;
        case slot_task_on_get_dir_error: task_on_get_dir_error(signal.API); break;
        case slot_task_on_get_file_error: task_on_get_file_error(signal.API); break;
        }
    }

    while(m_message_box_active == false && m_delayed_stat_queue.count() > 0) {
        EteraTaskSignalStat signal = m_delayed_stat_queue.dequeue();
        switch(signal.Slot) {
        case slot_task_on_put_ensure_success: task_on_put_ensure_success(signal.API, signal.Item); break;
        }
    }

    emit_delayed_tasks();
}
//----------------------------------------------------------------------------------------------

void WidgetDisk::emit_delayed_tasks() {
    if(m_delayed_task_queue.count() == 0)
        return;

    EteraTaskSignal signal = m_delayed_task_queue.dequeue();

    EteraAPI* api = signal.API;

    if(api->deleted() == true)
        return;

    switch(signal.Slot) {
    case slot_task_on_ls_error:
        api->ls(api->path(), api->preview(), api->crop(), api->offset(), api->limit());
        break;
    case slot_task_on_mkdir_error:
        api->mkdir(api->path());
        break;
    case slot_task_on_mkdir_stat_error:
        api->stat(api->path());
        break;
    case slot_task_on_copy_paste_error:
        api->cp(api->source(), api->target(), false);
        break;
    case slot_task_on_cut_paste_error:
        api->mv(api->source(), api->target(), false);
        break;
    case slot_task_on_copy_cut_paste_stat_error:
        api->stat(api->path(), m_preview_arg, true);
        break;
    case slot_task_on_rm_error:
        api->rm(api->path(), true);
        break;
    case slot_task_on_publish_error:
        api->publish(api->path());
        break;
    case slot_task_on_unpublish_error:
        api->unpublish(api->path());
        break;
    case slot_task_on_publish_stat_error:
        api->stat(api->path(), m_preview_arg, true);
        break;
    case slot_task_on_unpublish_stat_error:
        api->stat(api->path(), m_preview_arg, true);
        break;
    case slot_task_on_put_mkdir_error:
        api->mkdir(api->path());
        break;
    case slot_task_on_put_file_error:
        api->put(api->source(), api->target(), api->overwrite());
        break;
    case slot_task_on_put_stat_error:
        api->stat(api->path(), m_preview_arg, true);
        break;
    case slot_task_on_put_ensure_error:
        api->stat(api->path());
        break;
    case slot_task_on_put_rm_error:
        api->rm(api->path(), true);
        break;
    case slot_task_on_get_dir_error:
        api->ls(api->path(), api->preview(), api->crop(), api->offset(), api->limit());
        break;
    case slot_task_on_get_file_error:
        api->get(api->source(), api->target());
        break;
    }
}
//----------------------------------------------------------------------------------------------

void WidgetDisk::removeDelayed(const EteraAPI* api) {
    m_delayed_queue.erase(
        std::ranges::find(m_delayed_queue, api, &EteraTaskSignal::API));
    m_delayed_stat_queue.erase(
        std::ranges::find(m_delayed_stat_queue, api, &EteraTaskSignalStat::API));
    m_delayed_task_queue.erase(
        std::ranges::find(m_delayed_task_queue, api, &EteraTaskSignal::API));
}
//----------------------------------------------------------------------------------------------
