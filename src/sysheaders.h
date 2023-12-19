/*!
 * \file
 * \brief Файл для включения всех системных и библиотечных заголовочных файлов
 */

#ifndef _ekstertera_sysheaders_h_
#define _ekstertera_sysheaders_h_

//
// стандартные заголовочные файлы Qt
//

#include <QApplication>
#include <QBuffer>
#include <QCheckBox>
#include <QCloseEvent>
#include <QComboBox>
#include <QDateTime>
#include <QDebug>
#include <QDesktopServices>
#include <QDialog>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLibraryInfo>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QMimeData>
#include <QMultiMap>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QObject>
#include <QPainter>
#include <QProcessEnvironment>
#include <QProgressBar>
#include <QPushButton>
#include <QQueue>
#include <QScrollBar>
#include <QSet>
#include <QSettings>
#include <QSpacerItem>
#include <QSslCipher>
#include <QSslConfiguration>
#include <QStatusBar>
#include <QStringList>
#include <QStyledItemDelegate>
#include <QSystemTrayIcon>
#include <QTextCodec>
#include <QThread>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>
#include <QTranslator>
#include <QTreeView>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QWheelEvent>
#include <QWidget>

#if QT_VERSION >= 0x050000
// в Qt 5.x часть методов QUrl типа addQueryItem признаны obsolete
#include <QUrlQuery>

// Qt 5.x specific
#include <QMimeDatabase>
#include <QMimeType>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#endif

//
// windows
//

#ifdef Q_WS_WIN
// стандартные определения для Windows 7
#ifndef _WIN32_WINNT_WIN7
#define _WIN32_WINNT_WIN7 0x0601
#endif
#ifndef WINVER
#define WINVER _WIN32_WINNT_WIN7
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT WINVER
#endif
#ifndef NTDDI_WIN7
#define NTDDI_WIN7 0x06010000
#endif
#ifndef NTDDI_VERSION
#define NTDDI_VERSION NTDDI_WIN7
#endif
#ifndef _WIN32_IE_IE80
#define _WIN32_IE_IE80 0x0800
#endif
#ifndef _WIN32_IE
#define _WIN32_IE _WIN32_IE_IE80
#endif

#include <commctrl.h>
#include <commoncontrols.h>
#include <shellapi.h>
#include <windows.h>
#endif

//
// сторонние библиотеки
//

// https://github.com/gaudecker/qt-json
// #include "3dparty/json/json.h"

// бэкпорты из Qt 5.x
#if QT_VERSION < 0x050000
// https://qt.gitorious.org/qtplayground/mimetypes
#include "3dparty/qt5/mimetypes/qmimedatabase.h"
#include "3dparty/qt5/mimetypes/qmimetype.h"
#endif

//
// общие определения для проекта
//

// В Qt 5.x более нет макроса Q_WS_X11, который использовался для определения *nix подобных систем, определяем свои
#ifdef Q_WS_WIN
#define ETERA_WS_WIN
#endif

#ifdef Q_WS_MAC
#define ETERA_WS_MAC
#endif

#if defined(Q_OS_LINUX) || defined(Q_OS_UNIX)
#define ETERA_WS_X11
#endif

#if defined(ETERA_WS_X11) || defined(ETERA_WS_WIN)
#define ETERA_WS_X11_OR_WIN
#endif

// в Qt 5.x сломана иконка трея, необходима собственная реализация
// так же необходима реализация отдельного поведения для Ubuntu Unity
#ifndef ETERA_CUSTOM_TRAY_ICON
#if defined(ETERA_WS_X11) && (defined(ETERA_CUSTOM_TRAY_ICON_GTK) || defined(ETERA_CUSTOM_TRAY_ICON_UNITY))
#define ETERA_CUSTOM_TRAY_ICON
#endif
#else
#error "Use ETERA_CUSTOM_TRAY_ICON_GTK or ETERA_CUSTOM_TRAY_ICON_UNITY directive instead ETERA_CUSTOM_TRAY_ICON"
#endif

// конкатенация строк для макроса версии
#define ETERA_STR_EXPAND(token) #token
#define ETERA_STR(token)        ETERA_STR_EXPAND(token)

// версия (0.1.13)
#define ETERA_APP_NAME      "ekstertera"
#define ETERA_VERSION_MAJOR 0
#define ETERA_VERSION_MINOR 1
#define ETERA_VERSION_PATCH 13
#define ETERA_VERSION_NUM   ((ETERA_VERSION_MAJOR << 16) | (ETERA_VERSION_MINOR << 8) | (ETERA_VERSION_PATCH))
#define ETERA_VERSION       ETERA_STR(ETERA_VERSION_MAJOR) "." ETERA_STR(ETERA_VERSION_MINOR) "." ETERA_STR(ETERA_VERSION_PATCH)

// константы api
#define ETERA_API_USER_AGENT ETERA_APP_NAME "/" ETERA_VERSION
#define ETERA_API_BASE_URL   "https://cloud-api.yandex.net/v1/disk"
#define ETERA_API_APP_ID     "51cba73d70c343fd96f0765e1eeb0435"
#define ETERA_API_APP_SECRET "73cf4dacd4f74e7a97b77d036f90eb91"

#endif // _ekstertera_sysheaders_h_
