#include "tagitem.h"
#include "configtab.h"
#include "globbing.h"
#include <QSettings>

using ct = configtab;

TagItem::TagItem(const QVariant &data, TagItem *parent)
    : tag(data),
      parentItem(parent)
{}

TagItem::~TagItem()
{
    qDeleteAll(childItems);
}

TagItem *TagItem::child(int number)
{
    if (number < 0 || number >= childItems.size())
        return nullptr;
    return childItems.at(number);
}

int TagItem::childCount() const
{
    return childItems.count();
}

int TagItem::childNumber() const
{
    if (parentItem)
        return parentItem->childItems.indexOf(const_cast<TagItem*>(this));
    return 0;
}

QVariant TagItem::data(int column) const
{
    if (column == TAG_COLUMN){
        return tag;
    } else if (column == TYPE_COLUMN){
        return QVariant(type);
    }

    return QVariant();
}

bool TagItem::insertChildren(int position, int count)
{
    if (position < 0 || position > childItems.size())
        return false;

    for (int row = 0; row < count; ++row) {
        QVariant data;
        TagItem *item = new TagItem(data, this);
        childItems.insert(position, item);
    }

    return true;
}

TagItem *TagItem::parent()
{
    return parentItem;
}

bool TagItem::removeChildren(int position, int count)
{
    if (position < 0 || position + count > childItems.size())
        return false;

    for (int row = 0; row < count; ++row)
        delete childItems.takeAt(position);

    return true;
}


bool TagItem::setData(int column, const QVariant &value)
{
    if (column < 0 || column >= COLUMN_COUNT)
        return false;
    QSettings settings;

    if (column == TAG_COLUMN){
        const QString tag_text = value.toString();
        QVector<ct::color_setting> tag_colors = ct::get_tag_colors();
        fg_color = settings.value("special-tagcolor/default").value<QColor>();

        foreach(ct::color_setting tag_color,tag_colors)
            if (Globbing::glob_compare(tag_color.glob,tag_text))
                fg_color = tag_color.color;

        tag = value;
    } else if(column == TYPE_COLUMN){
        if (value.toInt() < 0 || value.toInt() >= TAG_TYPE_COUNT)
            return false;
        if (value.toInt() == SPECIAL_TAG)
            fg_color = settings.value("special-tagcolor/special",
                                      QColorConstants::LightGray).value<QColor>();
        else if (value.toInt() == INVALID_TAG)
            fg_color = settings.value("special-tagcolor/invalid",
                                      QColorConstants::Red).value<QColor>();
        else if (value.toInt() == NEW_TAG)
            fg_color = settings.value("special-tagcolor/new",
                                      QColor(254,192,51)).value<QColor>();

        type = (enum TAG_TYPE)value.toInt();

    }
    update_parent_color();
    return true;
}

void TagItem::update_parent_color()
{
    QSettings settings;
    QColor default_color = settings.value("special-tagcolor/default").value<QColor>();
    if (type == ALIAS_TAG && parentItem && fg_color != default_color && parentItem->fg_color == default_color){
        parentItem->fg_color = fg_color;
        parentItem->update_parent_color();
    }
}
