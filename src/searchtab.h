#ifndef SEARCHTAB_H
#define SEARCHTAB_H

#include <QWidget>

#include "tagtreemodel.h"
#include "filemodel.h"
#include "tab.h"

#include <QStandardItemModel>


namespace Ui {
class SearchTab;
}

class SearchTab : public Tab
{
    Q_OBJECT

public:
    explicit SearchTab(QTabWidget *parent = nullptr);
    SearchTab(QVector<PyrosFile*> &files,QTabWidget *parent = nullptr);
    SearchTab(QVector<QByteArray> &tags,QTabWidget *parent = nullptr);
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
    void create_viewer_tab(QVector<PyrosFile*>,int);
    void create_new_search_with_tags(QVector<QByteArray> tags);
    void new_duplicate_selector_tab(QVector<PyrosFile*>);
};

#endif // SEARCHTAB_H
