#ifndef THUMBNAILER_H
#define THUMBNAILER_H

#include <QObject>
#include <QVariant>
#include <QFutureWatcher>
#include <QPixmap>
struct PyrosFile;

class Thumbnailer : public QObject
{
    Q_OBJECT
public:
    class thumbnail_item{
    public:
        thumbnail_item(QByteArray path,QByteArray output_path,QByteArray mime);
        thumbnail_item(PyrosFile *file,int id = -1);
        thumbnail_item();

        int id;
        QPixmap thumbnail;
        QByteArray path;
        QByteArray output_path;
        QByteArray hash;
        QByteArray mime;
    };

    struct external_thumbnailer{
        QString cmd;
        QList<QByteArray> support_mimes;
    };


    static constexpr char thumbnail_extention[7] = ".thumb";

    Thumbnailer(QObject *parent = nullptr);


    static QVector <external_thumbnailer> loaded_thumbnailers;
    static bool video_thumbnailer(thumbnail_item &item);
    static bool image_thumbnailer(thumbnail_item &item);
    static bool cbz_thumbnailer(thumbnail_item &item);
    static bool external_thumbnailer(thumbnail_item &item);
    static thumbnail_item generate_thumbnail(thumbnail_item item);

    static void load_external_thumbnailers();

    static void delete_thumbnail(QByteArray hash);

    void start_thumbnailing(QVector<thumbnail_item> items);
    void stop_thumbnailing();
private:
    static void save_thumbnail(thumbnail_item &item);

    QFutureWatcher<thumbnail_item> *thumbnail_watcher;

private slots:
    void thumbnail_finished(int num);

signals:
    void thumbnail_generated(Thumbnailer::thumbnail_item item);

};

#endif // THUMBNAILER_H
