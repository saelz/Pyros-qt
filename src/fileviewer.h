#ifndef FILEVIEWER_H
#define FILEVIEWER_H

#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsPathItem>
#include <QPixmap>
#include <QImageReader>

#include <pyros.h>

#include "tab.h"

class zip_reader;

namespace Ui {
class FileViewer;
}

class FileViewer : public Tab
{
    Q_OBJECT

public:
    explicit FileViewer(QVector<PyrosFile*> files,int inital_pos = 0,QTabWidget *parent = nullptr);
    ~FileViewer();


private:
    Ui::FileViewer *ui;

    PyrosFile *m_pFile;

private slots:

    void set_file(PyrosFile *file);
    void select_tag_bar();
    void add_tag(QVector<QByteArray> tags);
    void remove_tag(QVector<QByteArray> tags);

signals:
    void file_deleted(QVector<QByteArray>);
    void hide_files(QVector<QByteArray>);
    void new_search_with_selected_tags(QVector<QByteArray>);

};

#endif // FILEVIEWER_H
