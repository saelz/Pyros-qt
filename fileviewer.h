#ifndef FILEVIEWER_H
#define FILEVIEWER_H

#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsPathItem>
#include <QPixmap>

#include <pyros.h>

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
    enum SCALE_TYPE{
        HEIGHT,
        WIDTH,
        BOTH,
        ORIGINAL,
    };

    enum ViewerType{
        IMAGE,
        GIF,
        MPV,
        TEXT,
        UNSUPPORTED,
    };



    Ui::FileViewer *ui;
    QPixmap m_img;
    QSize file_orignal_size;

    SCALE_TYPE scale_type = BOTH;
    ViewerType viewer_type = IMAGE;
    double zoom_level = 1;
    double zoom_increment = .25;
    int font_size = 12;

    PyrosFile *m_pFile;
    QVector<PyrosFile*> m_files;
    int position;

    bool eventFilter(QObject *obj,QEvent *event) override;

private slots:
    void update_fit(const QString &text);

    void zoom_in();
    void zoom_out();
    void set_scale();

    void select_tag_bar();
    void next_file();
    void prev_file();
    void delete_file();
    void add_tag(QVector<QByteArray> tags);
    void remove_tag(QVector<QByteArray> tags);
signals:
    void new_search_with_selected_tags(QVector<QByteArray>);
};

#endif // FILEVIEWER_H
