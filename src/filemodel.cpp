#include "filemodel.h"
#include "zip_reader.h"
#include "configtab.h"
#include "pyrosdb.h"

#include <QPixmap>
#include <QImage>
#include <iostream>
#include <functional>
#include <QProcess>
#include <QPainter>

using ct = configtab;

QVector<struct FileModel::external_thumbnailer> FileModel::loaded_thumbnailers;

FileModel::FileModel(QObject *parent) : QAbstractTableModel(parent)
{
    load_thumbnailers();
    thumbnailer = new QFutureWatcher<thumbnail_item>(this);

    connect(thumbnailer, &QFutureWatcher<QVariant>::resultReadyAt, this, &FileModel::displayThumbnail);
}

FileModel::~FileModel()
{
    clear();
    delete thumbnailer;
}

int FileModel::indexToNum(const QModelIndex &index) const
{
    int i = index.row()*m_columnCount + index.column();

    return i;
}
QModelIndex FileModel::numToIndex(const int num) const
{
    int cols = columnCount();

    if (num > 0 && cols > 0)
        return index(num/cols,num%cols);

    return index(0,0);

}

QVector<FileModel::file_item> FileModel::files() const
{
    return m_files;
}

PyrosFile *FileModel::file(const QModelIndex &index) const
{
    int position = indexToNum(index);

    if (position >= m_files.length() || position < 0)
        return nullptr;

    return m_files.at(position).pFile;
}

void FileModel::remove_file(const QModelIndex &index)
{
    int position = indexToNum(index);



    if (position >= m_files.length())
        return;

    Pyros_Close_File(m_files.at(position).pFile);
    m_files.remove(position);
}

void FileModel::clear()
{
    reset_thumbnailer();
    beginResetModel();

    foreach(file_item item, m_files)
        Pyros_Close_File(item.pFile);
    m_files.clear();

    endResetModel();
}

void FileModel::setFiles(PyrosList *files)
{
    if (files == nullptr)
        return;

    beginResetModel();
    for(size_t i = 0; i < files->length;i++)
        m_files.push_back({(PyrosFile*)files->list[i],QVariant()});
    endResetModel();

    Pyros_List_Free(files,nullptr);
    reset_thumbnailer();

}

void FileModel::setFilesFromVector(QVector<PyrosFile*> &files)
{
    beginResetModel();
    //m_files = files;
    foreach(PyrosFile *file,files)
        m_files.append({file,QVariant()});
    endResetModel();

    reset_thumbnailer();
}

void FileModel::reset_thumbnailer(){
    if (thumbnailer->isRunning()) {
        thumbnailer->cancel();
        thumbnailer->waitForFinished();
    }
    last_index = QModelIndex();
}

void FileModel::load_thumbnailers(){
    const char *thumbnailerdir = "/usr/share/thumbnailers";

    if (loaded_thumbnailers.length() > 0)
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
            loaded_thumbnailers.append(new_thumbnailer);
        }
    }

}

QVariant FileModel::external_thumbnailer(thumbnail_item item,QByteArray& thumbpath){

    foreach(struct external_thumbnailer nailer,loaded_thumbnailers){
        foreach(QByteArray mime,nailer.support_mimes){
            if (mime == item.mime){
                QString cmd  = nailer.cmd;
                cmd.replace("%i",item.path);
                cmd.replace("%u",item.path);
                cmd.replace("%o",thumbpath);
                cmd.replace("%s",ct::setting_value(ct::THUMBNAIL_SIZE).toString());

                QStringList cmd_list = cmd.split(' ');
                cmd = cmd_list.at(0);
                cmd_list.pop_front();
                if (cmd == "ffmpegthumbnailer" && cmd_list.back() == "-f")
                    cmd_list.pop_back();

                QProcess::execute(cmd,cmd_list);
                QPixmap pix;
                if (pix.load(thumbpath))
                    return QVariant(pix);
            }

        }
    }
    return QVariant();
}

void FileModel::delete_thumbnail(QByteArray hash){
    QByteArray thumbnail_dir = ct::setting_value(ct::THUMBNAIL_DIR).toByteArray();

    if (thumbnail_dir.at(0) == '~')
        thumbnail_dir = QDir::homePath().toUtf8() + thumbnail_dir.mid(1);

    QDir directory(thumbnail_dir);
    directory.setFilter(QDir::Dirs);
    QStringList thumbnail_size_dirs = directory.entryList();


    foreach(QString thumbnail_size,thumbnail_size_dirs){
        if (thumbnail_size == "." || thumbnail_size == "..")
            continue;

        QFile file(thumbnail_dir+"/"+thumbnail_size+"/"+hash+".thumb");
        file.remove();
    }

}

QVariant FileModel::internal_image_thumbnailer(thumbnail_item item,QByteArray& thumbpath){
    QPixmap pix;
    if (pix.load(item.path)){
        QPixmap newPix;
        if (pix.height() > pix.width())
            newPix = pix.scaledToHeight(ct::setting_value(ct::THUMBNAIL_SIZE).toInt(),Qt::SmoothTransformation);
        else
            newPix = pix.scaledToWidth(ct::setting_value(ct::THUMBNAIL_SIZE).toInt(),Qt::SmoothTransformation);

        newPix.save(thumbpath, "PNG");
        return QVariant(newPix);
    }
    return external_thumbnailer(item,thumbpath);
}

QVariant FileModel::internal_cbz_thumbnailer(thumbnail_item item,QByteArray& thumbpath)
{
    zip_reader reader;
    reader.read_file(item.path);
    if (!reader.isValid)
        return QVariant();

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

    QPixmap pi;
    pi.convertFromImage(surface);
    pi.save(thumbpath,"PNG");
    return pi;


}

FileModel::thumbnail_item FileModel::generateThumbnail (thumbnail_item item) {
    QPixmap pix1;

    QByteArray imgPath = ct::setting_value(ct::THUMBNAIL_DIR).toByteArray();

    if (imgPath.at(0) == '~')
        imgPath = QDir::homePath().toUtf8() + imgPath.mid(1);

    imgPath += "/"+ct::setting_value(ct::THUMBNAIL_SIZE).toByteArray()+"/";

    QDir dir(imgPath);
    if (!dir.exists())
        dir.mkpath(".");

    imgPath += item.hash+".thumb";

    if (pix1.load(imgPath)){
        item.thumbnail = pix1;
    } else {
        if (ct::setting_value(ct::USE_INTERNAL_IMAGE_THUMBNAILER).toBool() &&item.mime.startsWith("image/")){
            item.thumbnail = FileModel::internal_image_thumbnailer(item,imgPath);

        } else if (ct::setting_value(ct::USE_CBZ_THUMBNAILER).toBool() &&
               (!item.mime.compare("application/vnd.comicbook+zip") ||
                !item.mime.compare("application/zip"))){
            item.thumbnail = FileModel::internal_cbz_thumbnailer(item,imgPath);

        } else if (ct::setting_value(ct::USE_EXTERNAL_THUMBNAILER).toBool()){
            item.thumbnail = FileModel::external_thumbnailer(item,imgPath);
        }

        if (item.thumbnail.isNull())
            item.thumbnail = QPixmap(":/data/icons/nothumb.png");

    }
    return item;
}

void FileModel::load_thumbnails(QModelIndexList indexes)
{
    QVector<thumbnail_item> items;
    foreach(QModelIndex index,indexes){
            PyrosFile *pFile = file(index);

            if (pFile != nullptr){
                m_files[indexToNum(index)].thumbnail = QVariant();
                items.append({indexToNum(index),
                              QVariant(),
                              pFile->path,pFile->hash,pFile->mime});
            }
    }

    if (thumbnailer->isRunning()) {
        thumbnailer->cancel();
        thumbnailer->waitForFinished();
    }

    thumbnailer->setFuture(QtConcurrent::mapped(items,generateThumbnail));
}

void FileModel::append_thumbnail_items(QVector<thumbnail_item> &items,int start_row,int end_row,int column_count)
{
    for(int i = start_row; i <= end_row;i++){
        for	(int j = 0; j <= column_count;j++){
            QModelIndex current = index(i,j);
            PyrosFile *pFile = file(current);
            if (pFile == nullptr ||
                    m_files[indexToNum(current)].thumbnail != QVariant())
                continue;
            items.append({indexToNum(current),QVariant(),pFile->path,pFile->hash,pFile->mime});
        }
    }
}

void FileModel::load_thumbnails(QModelIndex topLeft,int rows)
{
    if (!topLeft.isValid())
        return;

    QVector<thumbnail_item> items;
    if (last_index == topLeft)
        return;

    last_index = topLeft;
    int row_end = topLeft.row()+rows+3;

    append_thumbnail_items(items,topLeft.row(),row_end,m_columnCount);

    if (items.length() == 0)
        return;

    append_thumbnail_items(items,row_end,row_end+2,m_columnCount);


    if (thumbnailer->isRunning()) {
        thumbnailer->cancel();
        thumbnailer->waitForFinished();
    }

    thumbnailer->setFuture(QtConcurrent::mapped(items,generateThumbnail));
}

void FileModel::displayThumbnail(int num)
{
    thumbnail_item item = thumbnailer->resultAt(num);
    int i;
    for (i = item.last_known_index; i >= 0; i--){
        if (i >= m_files.length())
            continue;
        if (item.path == m_files[i].pFile->path){
            m_files[i].thumbnail = item.thumbnail;
            break;
        }
    }
    QModelIndex current = numToIndex(i);

    emit dataChanged(current,current);
}

int FileModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return m_columnCount;
}

int FileModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    int row_count = m_files.length()/m_columnCount;

    if (m_files.length()%m_columnCount > 0)
        row_count++;

    return  row_count;
}

QVariant FileModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    int position = indexToNum(index);
    if (position >= m_files.length())
        return QVariant();


    if (role == Qt::DecorationRole){
        if (m_files.length() > position)
            return m_files[position].thumbnail;
    } else if (role == Qt::ForegroundRole) {
        return QColorConstants::Red;
    }
    return QVariant();
}

QVariant FileModel::headerData(int, Qt::Orientation, int) const
{
    //if (role == Qt::SizeHintRole)
    //    return QSize(1,1);

    return QVariant();
}

void FileModel::setColumnCount(int columns)
{
    if (columns <= 0)
        columns = 1;

    if (columns > m_columnCount){
        int inital_rowcount = rowCount(QModelIndex());

        beginInsertColumns(QModelIndex(),m_columnCount,columns-1);
        m_columnCount = columns;
        endInsertColumns();

        beginRemoveRows(QModelIndex(),rowCount(QModelIndex()),inital_rowcount-1);
        endRemoveRows();
    } else if (columns < m_columnCount){
        int inital_rowcount = rowCount(QModelIndex());

        beginRemoveColumns(QModelIndex(),columns,m_columnCount-1);
        m_columnCount = columns;
        endRemoveColumns();

        beginInsertRows(QModelIndex(),inital_rowcount,rowCount(QModelIndex())-1);
        endInsertRows();
    }

}

void FileModel::remove_excess_rows(int old_row_count)
{
    if (old_row_count == rowCount())
        return;

    beginRemoveRows(QModelIndex(),rowCount(),old_row_count-1);
    endRemoveRows();
}

void FileModel::unset_last_index()
{
    last_index = QModelIndex();
}
