#ifndef TAGITEM_H
#define TAGITEM_H

#include <QObject>
#include <QVariant>
#include <QVector>
#include <QColor>

class TagItem
{
public:
    explicit TagItem(const QVariant &data,TagItem* parent = nullptr);
    ~TagItem();

    enum COLUMNS{
        TAG_COLUMN = 0,
        TYPE_COLUMN,
        COLUMN_COUNT
    };

    enum TAG_TYPE{
        NORMAL_TAG = 0,
        ALIAS_TAG,
        SPECIAL_TAG,
        NEW_TAG,
        INVALID_TAG,
        TAG_TYPE_COUNT
    };

    TagItem *child(int number);
    int childCount() const;
    QVariant data(int column) const;
    TagItem *parent();
    int childNumber() const;
    bool insertChildren(int position, int count);
    bool setData(int column, const QVariant &value);

    bool removeChildren(int position, int count);

    QColor fg_color;
    QColor bg_color;
    QVariant tag;


private:
    QVector<TagItem*> childItems;
    enum TAG_TYPE type = NORMAL_TAG;
    TagItem *parentItem;
};

#endif // TAGITEM_H
