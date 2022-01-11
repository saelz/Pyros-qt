#include "tagitem.h"
#include "configtab.h"
#include "globbing.h"
#include <QSettings>

using ct = configtab;

TagItem::TagItem(const QVariant &data, TagItem *parent)
    : tag(data),
      parentItem(parent)
{
    highlight_color = ct::setting_value(ct::HIGHLIGHT_COLOR).value<QColor>();
    child_highlight_color = ct::setting_value(ct::CHILD_HIGHLIGHT_COLOR).value<QColor>();
}

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

QVariant TagItem::data(int role) const
{
    switch(role){
    case TagTreeModel::TAG_ROLE:
        return tag;
    case TagTreeModel::TYPE_ROLE:
        return type;
    case Qt::ForegroundRole:
        return fg_color;
    case Qt::BackgroundColorRole:
        if (highlighted)
            return highlight_color;
        else if (children_highlighted > 0)
            return child_highlight_color;
        else
            return QVariant();
    default:
        return QVariant();
    }
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


bool TagItem::setData(const QVariant &value,int role)
{
    QSettings settings;

    if (role == TagTreeModel::TAG_ROLE){
        const QString tag_text = value.toString();
        QVector<ct::color_setting> tag_colors = ct::get_tag_colors();
        fg_color = settings.value("special-tagcolor/default").value<QColor>();

        foreach(ct::color_setting tag_color,tag_colors)
            if (Globbing::glob_compare(tag_color.glob,tag_text))
                fg_color = tag_color.color;

        tag = value;
    } else if(role == TagTreeModel::TYPE_ROLE){
        if (value.toInt() < 0 || value.toInt() >= TagTreeModel::TAG_TYPE_COUNT)
            return false;
        if (value == TagTreeModel::SPECIAL_TAG)
            fg_color = settings.value("special-tagcolor/special",
                                      QColorConstants::LightGray).value<QColor>();
        else if (value == TagTreeModel::TEMP_TAG)
            fg_color = settings.value("special-tagcolor/unloaded",
                                      QColorConstants::LightGray).value<QColor>();
        else if (value == TagTreeModel::INVALID_TAG)
            fg_color = settings.value("special-tagcolor/invalid",
                                      QColorConstants::Red).value<QColor>();
        else if (value == TagTreeModel::NEW_TAG)
            fg_color = settings.value("special-tagcolor/new",
                                      QColor(254,192,51)).value<QColor>();

        type = value;

    }
    update_parent_color();
    return true;
}

void TagItem::update_parent_color()
{
    QSettings settings;
    QColor default_color = settings.value("special-tagcolor/default").value<QColor>();
    if (type == TagTreeModel::ALIAS_TAG && parentItem && fg_color != default_color && parentItem->fg_color == default_color){
        parentItem->fg_color = fg_color;
        parentItem->update_parent_color();
    }
}
