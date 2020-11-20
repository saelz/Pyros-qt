#include "tagitem.h"
#include <QSettings>

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
      switch (type) {
        default:
            return tag;
        case ALIAS_TAG:
            return  tag.toString() + " <A>";
        }
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
        fg_color = settings.value("special-tagcolor/default").value<QColor>();
        settings.beginGroup("tagcolor");
        QStringList colored_tags = settings.allKeys();
        foreach(QString colored_prefix,colored_tags){
            if (tag_text.startsWith(colored_prefix)){
                fg_color = settings.value(colored_prefix).value<QColor>();
            }
        }
        settings.endGroup();
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
                                      QColor("#fec033")).value<QColor>();

        type = (enum TAG_TYPE)value.toInt();
    }
    return true;
}
