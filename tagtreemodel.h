#ifndef TAGTREEMODEL_H
#define TAGTREEMODEL_H

#include <QObject>

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>

#include <pyros.h>

#include "tagitem.h"

#define PDB_PATH "/home/sam/.local/share/pyros/main/"


class TagTreeModel : public QAbstractItemModel
{
    Q_OBJECT
    //should keep list of tags
public:
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
private:

    TagItem *addChild(const QString &tag, TagItem *parent);
    bool generateModel(PyrosTag **pt,size_t cur,size_t max,TagItem *parent);
    TagItem *getItem(const QModelIndex &index) const;

    TagItem *rootItem;


};

#endif // TAGTREEMODEL_H
