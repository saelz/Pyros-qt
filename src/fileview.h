#ifndef FILEVIEW_H
#define FILEVIEW_H

#include <QObject>
#include <QWidget>
#include <QTableView>

#include "filemodel.h"

class FileDelegate;

class FileView : public QTableView
{
    Q_OBJECT
public:
    FileView(QWidget *parent = nullptr);

    void clear_tags();
    PyrosFile *file(const QModelIndex &index);
    QVector<PyrosFile*> files();
    FileModel *file_model;

private:
    FileDelegate *fd;
    QMenu *contextMenu_singlefile;
    QMenu *contextMenu_multiplefiles;

    QVector<QByteArray> m_tags;
    QTimer thumbtimer;

signals:
    void new_files(QVector<FileModel::file_item> files);
    void files_removed(QVector<QByteArray>);
    void new_duplicate_selector_tab(QVector<PyrosFile*>);

public slots:
    void search(QVector<QByteArray> tags);
    void set_files_from_vector(QVector<PyrosFile*> &files);
    void clear();
    void add_tag(QVector<QByteArray> tags);
    void remove_tag(QVector <QByteArray> tags);
    void remove_tag_from_search(QVector<QByteArray> tags);
    void refresh();
    void invertSelection();
    void hide_files_by_hash(QVector<QByteArray> hashes);
    void launch_timer();

private slots:

    void copy_path();
    void remove_file();
    void hide_file();
    void onCustomContextMenu(const QPoint &point);

    void get_visible();
    void regenerate_thumbnail();

    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

    void open_duplicate_menu();
};

#endif // FILEVIEW_H
