#include <QtConcurrent>
#include <QPixmap>
#include <QPainter>
#include <QVector>

#include <pyros.h>

#include "thumbnailer.h"
#include "video_thumbnailer.h"
#include "../configtab.h"
#include "../zip_reader.h"

using ct = configtab;

QVector<struct Thumbnailer::external_thumbnailer> Thumbnailer::loaded_thumbnailers = QVector<struct Thumbnailer::external_thumbnailer>();

constexpr char Thumbnailer::thumbnail_extention[7];
Thumbnailer::thumbnail_item::thumbnail_item(){}
Thumbnailer::thumbnail_item::thumbnail_item(QByteArray path,QByteArray output_path,QByteArray mime) :
    path(path),output_path(output_path),mime(mime)
{
    thumbnail = QPixmap();
}

Thumbnailer::thumbnail_item::thumbnail_item(PyrosFile *file, int id) :id(id)
{
    thumbnail = QPixmap();
    path = file->path;
    hash = file->hash;
    mime = file->mime;
    output_path = ct::setting_value(ct::THUMBNAIL_DIR).toByteArray();

    if (output_path.startsWith('~'))
        output_path = QDir::homePath().toUtf8()+output_path.mid(1);

    output_path += '/'+ct::setting_value(ct::THUMBNAIL_SIZE).toByteArray();

    QDir dir(output_path);
    if (!dir.exists())
        dir.mkpath(".");

    output_path += '/'+hash+Thumbnailer::thumbnail_extention;
}

Thumbnailer::Thumbnailer(QObject *parent) : QObject(parent)
{
    thumbnail_watcher = new QFutureWatcher<thumbnail_item>(this);
    load_external_thumbnailers();

    connect(thumbnail_watcher, &QFutureWatcher<QVariant>::resultReadyAt, this, &Thumbnailer::thumbnail_finished);
}

void Thumbnailer::stop_thumbnailing()
{
    if (thumbnail_watcher->isRunning()) {
        thumbnail_watcher->cancel();
    }
}

void Thumbnailer::start_thumbnailing(QVector<thumbnail_item> items)
{
    stop_thumbnailing();
    thumbnail_watcher->waitForFinished();
    thumbnail_watcher->setFuture(QtConcurrent::mapped(items,generate_thumbnail));
}

Thumbnailer::thumbnail_item Thumbnailer::generate_thumbnail(thumbnail_item item)
{
    QPixmap thumbnail;

    if (thumbnail.load(item.output_path)) {//thumbnail already exists
        item.thumbnail = thumbnail;
    } else {
        item.thumbnail = QPixmap();
        if (ct::setting_value(ct::USE_INTERNAL_IMAGE_THUMBNAILER).toBool() &&
                item.mime.startsWith("image/")){
            Thumbnailer::image_thumbnailer(item);

        } else if (ct::setting_value(ct::USE_CBZ_THUMBNAILER).toBool() &&
               (!item.mime.compare("application/vnd.comicbook+zip") ||
            !item.mime.compare("application/zip"))){
            Thumbnailer::cbz_thumbnailer(item);

        } else if (ct::setting_value(ct::USE_VIDEO_THUMBNAILER).toBool() &&
               (item.mime.startsWith("video/"))){
               Thumbnailer::video_thumbnailer(item);
        }

        if (item.thumbnail.isNull()){
            if (ct::setting_value(ct::USE_EXTERNAL_THUMBNAILER).toBool())
                Thumbnailer::external_thumbnailer(item);
            if (item.thumbnail.isNull())
                item.thumbnail = QPixmap(":/data/icons/nothumb.png");
        }  else{
            save_thumbnail(item);
        }
    }

    return item;
}
bool Thumbnailer::video_thumbnailer(thumbnail_item &item){
    QPixmap original_image = Video_thumbnailer::generate_thumbnail(item.path);

    if (original_image.isNull())
        return false;

    if (original_image.height() > original_image.width())
        item.thumbnail = original_image.scaledToHeight(ct::setting_value(ct::THUMBNAIL_SIZE).toInt(),Qt::SmoothTransformation);
    else
        item.thumbnail = original_image.scaledToWidth(ct::setting_value(ct::THUMBNAIL_SIZE).toInt(),Qt::SmoothTransformation);
    return true;
}

bool Thumbnailer::image_thumbnailer(thumbnail_item &item){
    QPixmap original_image;
    if (original_image.load(item.path)){
        if (original_image.height() > original_image.width())
            item.thumbnail = original_image.scaledToHeight(ct::setting_value(ct::THUMBNAIL_SIZE).toInt(),Qt::SmoothTransformation);
        else
            item.thumbnail = original_image.scaledToWidth(ct::setting_value(ct::THUMBNAIL_SIZE).toInt(),Qt::SmoothTransformation);
        return true;
    }
    return false;
}

bool Thumbnailer::cbz_thumbnailer(thumbnail_item &item)
{
    zip_reader reader;
    reader.read_file(item.path);
    if (!reader.isValid)
        return false;;

    QVector<QPixmap> pages;
    int page_count = (reader.file_count() < ct::setting_value(ct::CBZ_THUMB_PAGE_COUNT).toInt()) ? reader.file_count() : ct::setting_value(ct::CBZ_THUMB_PAGE_COUNT).toInt();

    int top_file_width = -1;
    int vspacing = 10;
    int hspacing;

    for (int i = 0;i < page_count;i++) {
        QPixmap img;
        QByteArray data;

        data = reader.get_file_data(i);
        if (data.isNull())
            continue;

        if (!img.loadFromData(data))
            continue;

        img = img.scaledToHeight(ct::setting_value(ct::THUMBNAIL_SIZE).toInt(),Qt::SmoothTransformation);
        if (!img.isNull()){
            if (top_file_width == -1)
                top_file_width = img.width();
            pages.push_front(img);
        }
    }

    QImage surface = QImage(ct::setting_value(ct::THUMBNAIL_SIZE).toInt(),ct::setting_value(ct::THUMBNAIL_SIZE).toInt(),QImage::Format_ARGB32_Premultiplied);
    surface.fill(Qt::transparent);
    QPainter p(&surface);
    p.setCompositionMode(QPainter::CompositionMode_Source);

    hspacing = ct::setting_value(ct::THUMBNAIL_SIZE).toInt()-top_file_width;
    if (pages.length() > 1)
        hspacing /= pages.length()-1;


    for(int i  = 0;i < pages.length();i++){
        const QPixmap *img = &pages.at(i);
        p.drawPixmap(hspacing*i,(pages.length()-(i+1))*vspacing,*img);
        //page border
        p.drawRect(hspacing*i,(pages.length()-(i+1))*vspacing,img->width(),img->height());
    }

    if (item.thumbnail.convertFromImage(surface))
        return true;
    return false;
}

void Thumbnailer::save_thumbnail(thumbnail_item &item)
{
    item.thumbnail.save(item.output_path,"PNG");
}

void Thumbnailer::load_external_thumbnailers()
{
    const char *thumbnailerdir = "/usr/share/thumbnailers";

    if (Thumbnailer::loaded_thumbnailers.length() > 0)
        return;

    QDirIterator it(thumbnailerdir);
    while (it.hasNext()) {
        QString filename = it.next();
        if (filename.endsWith(".thumbnailer")){
            QFile f(filename);
            if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
                continue;

            QByteArray line = f.readLine();

            if (!line.contains("[Thumbnailer Entry]"))
               continue;
            struct external_thumbnailer new_thumbnailer;

            while (!f.atEnd()) {
                line = f.readLine();
                if (line.startsWith("Exec=")){
                    QByteArray after = line.mid(5);
                    after.chop(1);
                    new_thumbnailer.cmd = after;
                } else if (line.startsWith("MimeType=")){
                    QByteArray after = line.mid(9);
                    after.chop(1);
                    new_thumbnailer.support_mimes = after.split(';');
                }
            }
            Thumbnailer::loaded_thumbnailers.append(new_thumbnailer);
        }
    }

}

bool Thumbnailer::external_thumbnailer(thumbnail_item &item)
{
    foreach(struct external_thumbnailer nailer,Thumbnailer::loaded_thumbnailers){
        foreach(QByteArray mime,nailer.support_mimes){
            if (mime == item.mime){
                QString cmd  = nailer.cmd;
                cmd.replace("%i",item.path);
                cmd.replace("%u",item.path);
                cmd.replace("%o",item.output_path);
                cmd.replace("%s",ct::setting_value(ct::THUMBNAIL_SIZE).toString());

                QStringList cmd_list = cmd.split(' ');
                cmd = cmd_list.at(0);
                cmd_list.pop_front();

                QProcess::execute(cmd,cmd_list);
                if (item.thumbnail.load(item.output_path))
                    return true;
            }
        }
    }
    return false;
}


void Thumbnailer::delete_thumbnail(QByteArray hash)
{
    QByteArray thumbnail_dir = ct::setting_value(ct::THUMBNAIL_DIR).toByteArray();

    if (thumbnail_dir.at(0) == '~')
        thumbnail_dir = QDir::homePath().toUtf8() + thumbnail_dir.mid(1);

    QDir directory(thumbnail_dir);
    directory.setFilter(QDir::Dirs);
    QStringList thumbnail_size_dirs = directory.entryList();


    foreach(QString thumbnail_size_dir,thumbnail_size_dirs){
        if (thumbnail_size_dir == "." || thumbnail_size_dir == "..")
            continue;

        QFile file(thumbnail_dir+"/"+thumbnail_size_dir+"/"+hash+Thumbnailer::thumbnail_extention);
        file.remove();
    }

}

void Thumbnailer::thumbnail_finished(int num)
{
    Thumbnailer::thumbnail_item item = thumbnail_watcher->resultAt(num);
    emit thumbnail_generated(item);
}
