#ifndef FILEVIEWER_H
#define FILEVIEWER_H

#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsPathItem>
#include <QPixmap>
#include <QImageReader>

#include <pyros.h>

#include "MediaViewer/mediaviewer.h"

class zip_reader;
//class QLabel;

namespace Ui {
class FileViewer;
}

class FileViewer : public QWidget
{
    Q_OBJECT

public:
    explicit FileViewer(QVector<PyrosFile*> files,int inital_pos = 0,QWidget *parent = nullptr);
    ~FileViewer();

    void set_file();

private:
    Ui::FileViewer *ui;

    PyrosFile *m_pFile;
    QVector<PyrosFile*> m_files;
    int position;

private slots:
    void update_fit(const QString &text);

    void select_tag_bar();
    void next_file();
    void prev_file();
    void delete_file();
    void add_tag(QVector<QByteArray> tags);
    void remove_tag(QVector<QByteArray> tags);

public slots:
    void hide_files(QVector<QByteArray> hashes);

signals:
    void file_deleted(QVector<QByteArray>);
    void new_search_with_selected_tags(QVector<QByteArray>);

};

#endif // FILEVIEWER_H
