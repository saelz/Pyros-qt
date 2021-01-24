#ifndef ZIP_READER_H
#define ZIP_READER_H

#include <QByteArray>
#include <QVector>

class QFile;

class zip_reader
{
    struct zipped_file{
        QByteArray filename;
        unsigned short compression_type;
        unsigned compressed_size;
        unsigned uncompressed_size;
        unsigned header_position;
    };

    const QByteArray SOCD_HEADER = (char[]){0x50,0x4b,0x01,0x02};
    enum SOCD_HEADER_DATA{
        file_compression_type    = 10,
        file_compressed_size     = 20,
        file_size                = 24,
        file_len_location        = 28,
        file_extra_len_location  = 30,
        file_comment_len_locaion = 32,
        file_starting_disk       = 34,
        file_location            = 42,
        filename_location        = 46,
    };

    const QByteArray LF_HEADER = (char[]){0x50,0x4b,0x03,0x04};
    enum LF_HEADER_DATA{
        lf_filename_len = 26,
        lf_extra_len    = 28,
        lf_filename     = 30,
    };

    const int buff_size = 1024;

    QByteArray m_file;
    QVector<zipped_file> m_files;

    template <typename T>
    T get_number(QFile &file,qint64 position);

    QByteArray get_bytes_at(QFile &file,qint64 position,int string_len);
    qint64 find_next(QFile &file,qint64 starting_pos,QByteArray header);

#ifdef ENABLE_ZLIB
    QByteArray uncompress(const QByteArray &data);
#endif
public:
    zip_reader();
    zip_reader(QByteArray file);
    int file_count();
    QByteArray get_file_data(int i);
    void read_file(QByteArray file);


    bool isValid = false;

};

#endif // ZIP_READER_H
