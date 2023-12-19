#include "widget_disk_item.h"
//----------------------------------------------------------------------------------------------
#include "utils/icon.h"
#include "utils/settings.h"
//----------------------------------------------------------------------------------------------

WidgetDiskItem::WidgetDiskItem(QListWidget* parent, const EteraItem& item, bool preview)
    : QListWidgetItem(parent, QListWidgetItem::UserType) {
    replaceItem(item, preview);
}
//----------------------------------------------------------------------------------------------

WidgetDiskItem::~WidgetDiskItem() {
    EteraIconProvider::instance()->cancelPreview(this);
}
//----------------------------------------------------------------------------------------------

bool WidgetDiskItem::operator<(const QListWidgetItem& other) const {
    const EteraItem* item = (static_cast<const WidgetDiskItem*>(&other))->item();

    if(m_item.type() == item->type())
        return QListWidgetItem::operator<(other);

    if(m_item.isDir() == true)
        return true;

    return false;
}
//----------------------------------------------------------------------------------------------

void WidgetDiskItem::update(bool preview) {
    EteraIconProvider* icon_provider = EteraIconProvider::instance();

    setText(m_item.name());

    if(preview == false || m_item.preview().isEmpty() == true)
        setIcon(icon_provider->icon(&m_item));
    else
        icon_provider->preview(this);
}
//----------------------------------------------------------------------------------------------

void WidgetDiskItem::revertText() {
    setText(m_item.name());
}
//----------------------------------------------------------------------------------------------

void WidgetDiskItem::replaceItem(const EteraItem& item, bool preview) {
    m_item = item;

    update(preview);
}
//----------------------------------------------------------------------------------------------
