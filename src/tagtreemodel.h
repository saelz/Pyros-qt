#ifndef TAGTREEMODEL_H
#define TAGTREEMODEL_H

#include <QObject>

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>

#include <pyros.h>

class TagItem;

class TagTreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    enum USER_ROLES{
        TAG_ROLE = Qt::DisplayRole,
        TYPE_ROLE = Qt::UserRole,
    };

    enum TAG_TYPE{
        NORMAL_TAG = 0,
        ALIAS_TAG,
        SPECIAL_TAG,
        NEW_TAG,
        INVALID_TAG,
        TEMP_TAG,
        TAG_TYPE_COUNT
    };


    TagTreeModel(const QString &column_title, QObject *parent = nullptr);
    ~TagTreeModel();


    QVariant data(const QModelIndex &index, int role) const override;

    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    bool insertRows(int position, int rows,
                    const QModelIndex &parent = QModelIndex()) override;
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    bool insertColumns(int position, int columns,
                       const QModelIndex &parent = QModelIndex()) override;

    bool removeRows(int position, int rows,
                    const QModelIndex &parent = QModelIndex()) override;

    bool removeColumns(int position, int columns, const QModelIndex &parent) override;

    bool setHeaderData(int section, Qt::Orientation orientation,
                              const QVariant &value, int role = Qt::EditRole) override;

    bool update_tag_type(const QModelIndex &index, enum TAG_TYPE type);

    void set_tag_highlight(const QModelIndex &index,bool highlight);
private:
    void set_child_tag_highlight(const QModelIndex &index,bool highlight);

    TagItem *addChild(const QString &tag, TagItem *parent);
    bool generateModel(PyrosTag **pt,size_t cur,size_t max,TagItem *parent);
    TagItem *getItem(const QModelIndex &index) const;

    TagItem *rootItem;


};

#endif // TAGTREEMODEL_H
