#ifndef FILEMODEL_H
#define FILEMODEL_H

#include <QAbstractTableModel>
#include <QVariant>
#include <QtConcurrent/QtConcurrent>

#include <pyros.h>
#include "thumbnailer.h"

class FileModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    FileModel(QObject *parent = nullptr);
    ~FileModel();

    struct file_item{
        PyrosFile* pFile;
        QVariant thumbnail;
    };

    QVector<file_item> files() const;

    void setFiles(PyrosList *files);
    void setFilesFromVector(QVector<PyrosFile*> &files);

    PyrosFile *file(const QModelIndex &index) const;

    void clear();

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    QVariant headerData(int /* section */, Qt::Orientation /* orientation */,
                        int ) const override;

    void remove_file(const QModelIndex &index);
    int indexToNum(const QModelIndex &index) const;
    QModelIndex numToIndex(const int num) const;

    void load_thumbnails(QModelIndex topLeft,int rows);
    void load_thumbnails(QModelIndexList indexes);

    void setColumnCount(int columns);
    void remove_excess_rows(int old_row_count);
    void unset_last_index();

signals:
    void sourceChanged();

private:

    int m_columnCount = 4;
    QModelIndex last_index = QModelIndex();

    QVector<file_item> m_files;

    void append_thumbnail_items(QVector<Thumbnailer::thumbnail_item> &items,int start_row,int end_row,int column_count);

    Thumbnailer *thumbnailer;
    void reset_thumbnailer();


private slots:
    void displayThumbnail(Thumbnailer::thumbnail_item item);

};

#endif // FILEMODEL_H
