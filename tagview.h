#ifndef TAGVIEW_H
#define TAGVIEW_H
#include "tagtreemodel.h"

#include <QWidget>
#include <QTreeView>
#include <QMenu>

class  QSortFilterProxyModel;

class TagView : public QTreeView
{
    Q_OBJECT
public:
    TagView(QWidget *parent = nullptr);
    ~TagView();

    QMenu* contextMenu;

    void clear();
    bool loadModelFromTag(QByteArray tag, const QModelIndex index,PyrosDB *pyrosDB);
    bool generateModel(PyrosTag **pt,size_t cur,size_t max,const QModelIndex index,TagItem::TAG_TYPE type = TagItem::NORMAL_TAG);

    void setTagsFromFile(PyrosFile *file);

    void setTagType(int type);

    int tag_type = PYROS_FILE_EXT;
    TagTreeModel *tag_model;

private:
    QVector<QByteArray> get_selected_tags();
    QVector<QByteArray> create_tag_dialog(QByteArray title);
    QSortFilterProxyModel *sort_model;

    void add_tag_as_child(TagItem::TAG_TYPE type,QString tag);
public slots:
    void add_tags(QVector<QByteArray> tags);
    void remove_tag();

private slots:

    void remove_ext();

    void add_alias();
    void add_parent();
    void add_child();

    void copy_tag();

    void onCustomContextMenu(const QPoint &point);
    void create_search_with_selected_tags();


signals:
    void removeTag(QVector <QByteArray>);
    void new_search_with_selected_tags(QVector <QByteArray>);
};

#endif // TAGVIEW_H
