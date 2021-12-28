#ifndef TAGITEM_H
#define TAGITEM_H

#include <QObject>
#include <QVariant>
#include <QVector>
#include <QColor>

#include "tagtreemodel.h"

class TagItem
{
public:
    explicit TagItem(const QVariant &data,TagItem* parent = nullptr);
    ~TagItem();

    TagItem *child(int number);
    int childCount() const;
    QVariant data(int role) const;
    TagItem *parent();
    int childNumber() const;
    bool insertChildren(int position, int count);
    bool setData(const QVariant &value,int role);

    bool removeChildren(int position, int count);

    void update_parent_color();

private:
    QVector<TagItem*> childItems;
    QVariant tag;
    QVariant type = TagTreeModel::NORMAL_TAG;
    TagItem *parentItem;
    QColor fg_color;

};

#endif // TAGITEM_H
