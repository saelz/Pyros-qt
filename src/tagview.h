#ifndef TAGVIEW_H
#define TAGVIEW_H

#include <QWidget>
#include <QTreeView>
#include <QMenu>

class TagTreeModel;
struct PyrosTag;
struct PyrosFile;
struct PyrosList;

class  QSortFilterProxyModel;

class TagView : public QTreeView
{
    Q_OBJECT
public:
    TagView(QWidget *parent = nullptr);
    ~TagView();

    QMenu* contextMenu;

    void clear();
    bool generateModel(PyrosTag **pt,size_t cur,size_t max,const QModelIndex index,int tag_type);

    void setTagsFromFile(PyrosFile *file);

    void setTagType(int type);

    void append_search_options_to_contenxt_menu();

    int tag_type;
    TagTreeModel *tag_model;

private:
    QVector<QByteArray> get_selected_tags();
    QVector<QByteArray> create_tag_dialog(QByteArray title);
    QSortFilterProxyModel *sort_model;
    QByteArray file_hash;

    bool add_unloaded_tag_to_model(QString tag);

    void add_related_tags_to_view_recursively(QVector<QByteArray> tags,QVector<QByteArray> related_tags,uint type,QModelIndex parent);
    void remove_related_tags_from_view_recursively(QVector<QByteArray> tag_pairs,QModelIndex parent,bool tag_found);
public slots:
    void remove_tag_from_view(QVector<QByteArray> hashes,QVector<QByteArray> tags);
    void add_tags_by_hash(QVector<QByteArray> hashes,QVector<QByteArray> tags);
    void add_tags(QVector<QByteArray> tags);
    void remove_tag();
    void add_related_tags_to_view(QVector<QByteArray> tags,QVector<QByteArray> related_tags,uint type);
    void remove_related_tags_from_view(QVector<QByteArray> tag_pairs);

    void highlight_similar_tags(const QString &text);
private:
    void highlight_similar_tags_recursively(const QString &text,const QModelIndex parent);
private slots:

    void replace_temp_tags(QVector<PyrosList*> related_tags,QVector<QByteArray> unfound_tags);

    void remove_relationship();

    void add_alias();
    void add_parent();
    void add_child();

    void copy_tag();

    void onCustomContextMenu(const QPoint &point);
    void create_search_with_selected_tags();

    void add_selected_tags_to_search();
    void filter_selected_tags_from_search();
signals:
    void removeTag(QVector <QByteArray>);
    void new_search_with_selected_tags(QVector <QByteArray>);
    void add_tag_to_current_search(QVector<QByteArray>);
};

#endif // TAGVIEW_H
