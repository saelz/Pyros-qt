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
    ~FileView();

    //void set_search_from_vector(PyrosDB *pyrosDB, QVector<char*> *hashes);
    void clear_tags();
    PyrosFile *file(const QModelIndex &index);
    QVector<PyrosFile*> files();
    FileModel *file_model;

private:
    FileDelegate *fd;
    QMenu *contextMenu;

    QVector<QByteArray> m_tags;
    QTimer thumbtimer;

signals:
    void new_files(QVector<FileModel::file_item> files);

public slots:
    void search(QVector<QByteArray> tags);
    void set_files_from_vector(QVector<PyrosFile*> &files);
    void add_tag(QVector<QByteArray> tags);
    void remove_tag(QVector <QByteArray> tags);
    void remove_tag_from_search(QVector<QByteArray> tags);
    void refresh();
    void invertSelection();
private slots:

    void copy_path();
    void remove_file();
    void hide_file();
    void onCustomContextMenu(const QPoint &point);

    void get_visible();

    void resizeEvent(QResizeEvent *event) override;
    void launch_timer();
};

#endif // FILEVIEW_H
