#ifndef FILEMODEL_H
#define FILEMODEL_H

#include <QAbstractTableModel>
#include <QVariant>
#include <QtConcurrent/QtConcurrent>

#include <pyros.h>

class FileModel : public QAbstractTableModel
{
    Q_OBJECT
    struct thumbnail_item{
        int last_known_index;
        QVariant thumbnail;
        QByteArray path;
        QByteArray hash;
        QByteArray mime;
    };


public:
    FileModel(QObject *parent = nullptr);
    ~FileModel();

    struct file_item{
        PyrosFile* pFile;
        QVariant thumbnail;
    };

    struct external_thumbnailer{
        QString cmd;
        QList<QByteArray> support_mimes;
    };

    static QVector <external_thumbnailer> loaded_thumbnailers;
    static QVariant internal_image_thumbnailer(thumbnail_item item,QByteArray &thumbpath);
    static QVariant internal_cbz_thumbnailer(thumbnail_item item,QByteArray &thumbpath);
    static QVariant external_thumbnailer(thumbnail_item item,QByteArray &thumbpath);

    static void delete_thumbnail(QByteArray hash);

    static FileModel::thumbnail_item generateThumbnail (thumbnail_item item);

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

    void startThumbnailer(QVector<QModelIndex> *visible_files);
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

    void append_thumbnail_items(QVector<thumbnail_item> &items,int start_row,int end_row,int column_count);

    QFutureWatcher<thumbnail_item> *thumbnailer;
    void load_thumbnailers();
    void reset_thumbnailer();


private slots:
    void displayThumbnail(int num);

};

#endif // FILEMODEL_H
