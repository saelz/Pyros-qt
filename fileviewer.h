#ifndef FILEVIEWER_H
#define FILEVIEWER_H

#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsPathItem>
#include <QPixmap>
#include <QImageReader>

#include <pyros.h>

class zip_reader;
class QLabel;

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

    enum SCALE_TYPE{
        HEIGHT,
        WIDTH,
        BOTH,
        ORIGINAL,
    };

    class Viewer{
    protected:
        QLabel *m_label;
        int boundry_width = 0;
        int boundry_height = 0;
        FileViewer::SCALE_TYPE scale_type = BOTH;
    public:
        Viewer(QLabel *label);
        Viewer();
        virtual ~Viewer(){};
        virtual void set_file(char *filepath) = 0;
        virtual void resize(int width,int height,
                              FileViewer::SCALE_TYPE scale);
        virtual void update_size(){};
        virtual void zoom_in(){};
        virtual void zoom_out(){};

        virtual void next_page(){};
        virtual void prev_page(){};

        virtual QString get_info(){return "";}
    };

private:

    Viewer *viewer = nullptr;

    Ui::FileViewer *ui;

    SCALE_TYPE scale_type = BOTH;

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

    void cbz_next_page();
    void cbz_prev_page();

    void set_file_info(QString string);

public slots:
    void hide_files(QVector<QByteArray> hashes);

signals:
    void file_deleted(QVector<QByteArray>);
    void new_search_with_selected_tags(QVector<QByteArray>);
};

#endif // FILEVIEWER_H
