#include "tagtreemodel.h"
#include "tagitem.h"

#include <QtWidgets>


TagTreeModel::TagTreeModel(const QString &tag,QObject *parent)
    : QAbstractItemModel(parent)
{
    QVariant rootData(tag);

    rootItem = new TagItem(rootData);
}

TagTreeModel::~TagTreeModel()
{
    delete rootItem;
}

int TagTreeModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 1;
}

QVariant TagTreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    TagItem *item = getItem(index);

    return item->data(role);
}

TagItem *TagTreeModel::getItem(const QModelIndex &index) const
{
    if (index.isValid()){
       TagItem *item = static_cast<TagItem*>(index.internalPointer());
       return item;
    }

    return rootItem;
}

QModelIndex TagTreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid() && parent.column() != 0)
        return QModelIndex();

    TagItem *parentItem = getItem(parent);
    if (!parentItem)
        return QModelIndex();

    TagItem *childItem = parentItem->child(row);

    if (childItem)
        return createIndex(row,column,childItem);

    return QModelIndex();
}

QModelIndex TagTreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    TagItem *childItem = getItem(index);
    TagItem *parentItem = childItem ? childItem->parent() : nullptr;

    if (parentItem == rootItem || !parentItem)
        return QModelIndex();

    return createIndex(parentItem->childNumber(), 0, parentItem);
}

int TagTreeModel::rowCount(const QModelIndex &parent) const
{
    const TagItem *parentItem = getItem(parent);

    if (parentItem != nullptr)
        return parentItem->childCount();
    else
        return 0;
}


TagItem *TagTreeModel::addChild(const QString &tag, TagItem *parent)
{
    if (!tag.isEmpty()){
        parent->insertChildren(0,1);
        TagItem *child = parent->child(0);
        child->setData(tag,TAG_ROLE);
        return child;
    }
    return nullptr;
}

QVariant TagTreeModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    Q_UNUSED(section);
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return rootItem->data(TAG_ROLE);

    return QVariant();
}

bool TagTreeModel::insertRows(int position, int rows, const QModelIndex &parent)
{
    TagItem *parentItem = getItem(parent);
    if (!parentItem)
        return false;

    beginInsertRows(parent, position, position + rows -1);
    bool success = parentItem->insertChildren(position,
                                                    rows);
    endInsertRows();

    return success;
}

bool TagTreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || (role != TAG_ROLE && role != TYPE_ROLE))
        return false;

    TagItem *item = getItem(index);

    bool result = item->setData(value, role);

    if (result && item->data(index.column()).isValid())
        emit dataChanged(index, index, {Qt::DisplayRole});

    return result;
}

bool TagTreeModel::update_tag_type(const QModelIndex &index, TAG_TYPE type)
{
    TagItem *item = getItem(index);

    if (type == NORMAL_TAG && item->data(TYPE_ROLE) != type)
        item->setData(item->data(TAG_ROLE),TAG_ROLE);

    bool result = item->setData(type,TYPE_ROLE);

    if (result && item->data(index.column()).isValid())
        emit dataChanged(index, index, {Qt::DisplayRole});

    return result;
}

Qt::ItemFlags TagTreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return  Qt::ItemIsSelectable | QAbstractItemModel::flags(index);
}

bool TagTreeModel::insertColumns(int position, int columns, const QModelIndex &parent)
{
    Q_UNUSED(position);
    Q_UNUSED(columns);
    Q_UNUSED(parent);
    return false;
}

bool TagTreeModel::removeRows(int position, int rows, const QModelIndex &parent)
{
    TagItem *parentItem = getItem(parent);
    if (!parentItem)
        return false;

    beginRemoveRows(parent, position, position + rows - 1);
    const bool success = parentItem->removeChildren(position, rows);
    endRemoveRows();

    return success;
}

bool TagTreeModel::removeColumns(int position, int columns, const QModelIndex &parent)
{
    Q_UNUSED(position);
    Q_UNUSED(columns);
    Q_UNUSED(parent);
    return false;
}

bool TagTreeModel::setHeaderData(int section, Qt::Orientation orientation,
                              const QVariant &value, int role)
{
    if (role != Qt::EditRole || orientation != Qt::Horizontal)
        return false;

    const bool result = rootItem->setData(value,TAG_ROLE);

    if (result)
        emit headerDataChanged(orientation, section, section);

    return result;
}

void TagTreeModel::set_child_tag_highlight(const QModelIndex &index, bool highlight)
{
    TagItem *item = getItem(index);
    if (item != rootItem){
        if (highlight)
            item->children_highlighted++;
        else
            item->children_highlighted--;

        if (item->children_highlighted <= 1)
            emit dataChanged(index, index, {Qt::BackgroundColorRole});
        set_child_tag_highlight(index.parent(),highlight);
    }
}

void TagTreeModel::set_tag_highlight(const QModelIndex &index, bool highlight)
{
    TagItem *item = getItem(index);
    if (item != rootItem && item->highlighted != highlight){
        item->highlighted = highlight;
        emit dataChanged(index, index, {Qt::BackgroundColorRole});
        set_child_tag_highlight(index.parent(),highlight);
    }
}

