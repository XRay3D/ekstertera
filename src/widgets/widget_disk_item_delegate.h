/*!
 * \file
 * \brief Делегат для ручной отрисовки элемента виджета проводника Яндекс.Диск
 */

#ifndef _ekstertera_widgets_widget_disk_item_delegate_h_
#define _ekstertera_widgets_widget_disk_item_delegate_h_

#include "sysheaders.h"

/*!
 * \brief Делегат для ручной отрисовки элемента виджета проводника Яндекс.Диск
 */
class WidgetDiskItemDelegate : public QStyledItemDelegate {
public:
    WidgetDiskItemDelegate(QObject* parent = nullptr);
    ~WidgetDiskItemDelegate();

    // QStyledItemDelegate
public:
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;

private:
    int adjust_dx; /*!< \brief Отсуп у горизонтальных границ */
    int adjust_dy; /*!< \brief Отсуп у вертикальных границ   */
};

#endif // _ekstertera_widgets_widget_disk_item_delegate_h_
