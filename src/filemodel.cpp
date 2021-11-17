#include "filemodel.h"
#include "zip_reader.h"
#include "configtab.h"
#include "pyrosdb.h"
#include "thumbnailer.h"

#include <QPixmap>
#include <QImage>
#include <iostream>
#include <functional>
#include <QProcess>
#include <QPainter>

using ct = configtab;


FileModel::FileModel(QObject *parent) : QAbstractTableModel(parent)
{
    thumbnailer = new Thumbnailer(this);
    connect(thumbnailer, &Thumbnailer::thumbnail_generated, this, &FileModel::displayThumbnail);
}

FileModel::~FileModel()
{
    clear();
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

    foreach(PyrosFile *file,files)
        m_files.append({file,QVariant()});
    endResetModel();

    reset_thumbnailer();
}

void FileModel::reset_thumbnailer(){
    thumbnailer->stop_thumbnailing();
    last_index = QModelIndex();
}

void FileModel::load_thumbnails(QModelIndexList indexes)
{
    QVector<Thumbnailer::thumbnail_item> items;
    foreach(QModelIndex index,indexes){
        PyrosFile *pFile = file(index);

        if (pFile != nullptr){
            m_files[indexToNum(index)].thumbnail = QVariant();
            items.append({indexToNum(index),
                  QPixmap(),
                  pFile->path,pFile->hash,pFile->mime});
        }
    }

    thumbnailer->start_thumbnailing(items);
}

void FileModel::append_thumbnail_items(QVector<Thumbnailer::thumbnail_item> &items,int start_row,int end_row,int column_count)
{
    for(int i = start_row; i <= end_row;i++){
        for	(int j = 0; j <= column_count;j++){
            QModelIndex current = index(i,j);
            PyrosFile *pFile = file(current);
            if (pFile == nullptr ||
                    m_files[indexToNum(current)].thumbnail != QVariant())
                continue;
            items.append({indexToNum(current),QPixmap(),pFile->path,pFile->hash,pFile->mime});
        }
    }
}

void FileModel::load_thumbnails(QModelIndex topLeft,int rows)
{
    if (!topLeft.isValid())
        return;

    QVector<Thumbnailer::thumbnail_item> items;
    if (last_index == topLeft)
        return;

    last_index = topLeft;
    int row_end = topLeft.row()+rows+3;

    append_thumbnail_items(items,topLeft.row(),row_end,m_columnCount);

    if (items.length() == 0)
        return;

    append_thumbnail_items(items,row_end,row_end+2,m_columnCount);

    thumbnailer->start_thumbnailing(items);
}

void FileModel::displayThumbnail(Thumbnailer::thumbnail_item item)
{
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
