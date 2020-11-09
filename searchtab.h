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
    ~SearchTab();

private:
    Ui::SearchTab *ui;
    void init();
    void create_title(QVector<QByteArray> tags);


private slots:
    void set_file_count(QVector<FileModel::file_item>);
    void create_new_viewer_tab(const QModelIndex &index);
    void set_tag_view(const QModelIndex &current, const QModelIndex &previous);
    void clear();
    void clear_file_data();
    void select_search_bar();
    void select_tag_bar();
    void select_file_view();
    void set_bottom_bar(const QItemSelection &selected, const QItemSelection &deselected);

signals:
    void set_title(QString ,QWidget *);
    void create_viewer_tab(QVector<PyrosFile*>,int);

};

#endif // SEARCHTAB_H
