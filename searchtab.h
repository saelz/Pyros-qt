#ifndef SEARCHTAB_H
#define SEARCHTAB_H

#include <QWidget>

#include "tagtreemodel.h"
#include "filemodel.h"

#include <QStandardItemModel>


namespace Ui {
class SearchTab;
}

class SearchTab : public QWidget
{
    Q_OBJECT

public:
    explicit SearchTab(QWidget *parent = nullptr);
    SearchTab(QVector<PyrosFile*> &files,QWidget *parent = nullptr);
    SearchTab(QVector<QByteArray> &tags,QWidget *parent = nullptr);
    ~SearchTab();

private:
    Ui::SearchTab *ui;
    void init();
    void create_title(QVector<QByteArray> tags);
    void set_loading_screen(QString text);
    void show_results();


private slots:
    void set_file_count(QVector<FileModel::file_item>);
    void create_new_viewer_tab(const QModelIndex &index);
    void clear();
    void clear_file_data();
    void select_search_bar();
    void select_tag_bar();
    void select_file_view();
    void set_bottom_bar(const QItemSelection &selected, const QItemSelection &deselected);

signals:
    void file_deleted(QVector<QByteArray>);
    void set_title(QString ,QWidget *);
    void create_viewer_tab(QVector<PyrosFile*>,int);
    void create_new_search_with_tags(QVector<QByteArray> tags);
    void hide_files_by_hash(QVector<QByteArray> hashes);
    void new_duplicate_selector_tab(QVector<PyrosFile*>);
};

#endif // SEARCHTAB_H
