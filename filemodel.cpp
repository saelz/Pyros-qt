#include "filemodel.h"

#include <QPixmap>
#include <QImage>
#include <iostream>
#include <functional>
#include <QProcess>

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
                //qDebug() << "using thumbnailer:" << nailer.cmd;
                QString cmd  = nailer.cmd;
                cmd.replace("%i",item.path);
                cmd.replace("%u",item.path);
                cmd.replace("%o",thumbpath);
                cmd.replace("%s","256");

                QStringList cmd_list = cmd.split(' ');
                cmd = cmd_list.at(0);
                cmd_list.pop_front();
                if (cmd == "ffmpegthumbnailer")
                    cmd_list.pop_back();

                /*QProcess::execute("ffmpegthumbnailer", QStringList() <<
                                  "-s" << "256" << "-i" << pFile->path<< "-o" << imgPath );*/

                QProcess::execute(cmd,cmd_list);
                QPixmap pix;
                if (pix.load(thumbpath))
                    return QVariant(pix);
            }

        }
    }
    return QVariant("no thumbnail");
}

QVariant FileModel::generic_image_thumbnailer(thumbnail_item item,QByteArray& thumbpath){
    QPixmap pix;
    if (pix.load(item.path)){
        QPixmap newPix;
        if (pix.height() > pix.width())
            newPix = pix.scaledToHeight(256,Qt::SmoothTransformation);
        else
            newPix = pix.scaledToWidth(256,Qt::SmoothTransformation);

        QFile file(thumbpath);
        file.open(QIODevice::WriteOnly);
        newPix.save(&file, "PNG");

        qDebug() << thumbpath << '\n';

        return QVariant(newPix);
    }
    return external_thumbnailer(item,thumbpath);
}

FileModel::thumbnail_item FileModel::generateThumbnail (thumbnail_item item) {
        QPixmap pix1;

        QByteArray imgPath(item.path);
        imgPath += ".256";
        //qDebug("THUMBNAILING: %s",imgPath.data());

        if (pix1.load(imgPath)){
            item.thumbnail = pix1;
        } else {
            if (item.mime.startsWith("image/")){
                item.thumbnail = FileModel::generic_image_thumbnailer(item,imgPath);
            } else{
                item.thumbnail = FileModel::external_thumbnailer(item,imgPath);
            }
        }
        return item;
}

void FileModel::load_thumbnails(QModelIndex topLeft,int rows)
{
    if (!topLeft.isValid())
        return;

    int i,j;
    QModelIndex current;
    QVector<thumbnail_item> items;
    if (last_index == topLeft)
        return;
    last_index = topLeft;
    int row_end = topLeft.row()+rows+3;

    for	(i = topLeft.row(); i <= row_end;i++){
        for	(j = 0; j <= m_columnCount;j++){
            current = index(i,j);
            PyrosFile *pFile = file(current);
            if (pFile == nullptr ||
                    m_files[indexToNum(current)].thumbnail != QVariant())
                continue;
            items.append({indexToNum(current),QVariant(),pFile->path,pFile->mime});
        }
    }

    if (items.length() == 0)
        return;
    for	(i = row_end; i <= row_end+2;i++){
        for	(j = 0; j <= m_columnCount;j++){
            current = index(i,j);
            PyrosFile *pFile = file(current);
            if (pFile == nullptr ||
                    m_files[indexToNum(current)].thumbnail != QVariant())
                continue;
            items.append({indexToNum(current),QVariant(),pFile->path,pFile->mime});
        }
    }


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
    QModelIndex current;
    if (i == 0)
        current = index(0,0);
    else
        current = index(i/m_columnCount,i%m_columnCount);
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
