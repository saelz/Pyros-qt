#ifndef THUMBNAILER_H
#define THUMBNAILER_H

#include <QObject>
#include <QVariant>
#include <QFutureWatcher>
#include <QPixmap>

class Thumbnailer : public QObject
{
    Q_OBJECT
public:
    struct thumbnail_item{
        int last_known_index;
        QPixmap thumbnail;
        QByteArray path;
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
    static QPixmap image_thumbnailer(thumbnail_item);
    static QPixmap cbz_thumbnailer(thumbnail_item);
    static QPixmap external_thumbnailer(thumbnail_item,QByteArray &thumbpath);
    static thumbnail_item generate_thumbnail(thumbnail_item item);

    static void load_external_thumbnailers();

    static void delete_thumbnail(QByteArray hash);

    void start_thumbnailing(QVector<thumbnail_item> items);
    void stop_thumbnailing();
private:
    static void save_thumbnail(QPixmap thumbnail,QByteArray &thumbpath);

    QFutureWatcher<thumbnail_item> *thumbnail_watcher;

private slots:
    void thumbnail_finished(int num);

signals:
    void thumbnail_generated(thumbnail_item item);

};

#endif // THUMBNAILER_H
