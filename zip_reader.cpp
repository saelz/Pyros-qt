#include "zip_reader.h"

#include <zlib.h>

#include <QByteArray>
#include <QFile>
#include <QtEndian>

zip_reader::zip_reader(){}
zip_reader::zip_reader(QByteArray file)
{
    read_file(file);
}

QByteArray zip_reader::get_string_at(QFile &file,qint64 position,int string_len)
{
    file.seek(position);
    char *string = new char[string_len];
    if (file.read(string,string_len) < 0){
        isValid = false;
        qDebug("Error reading data at 0x%llx",position);
        return QByteArray();
    }

    QByteArray result(string,string_len);
    delete[] string;
    return result;
}

template <typename T>
T zip_reader::get_number(QFile &file,qint64 position)
{
    file.seek(position);
    T result;
    char  bytes[sizeof(result)];
    if (file.read(bytes,sizeof(result)) < 0){
        isValid = false;
        qDebug("unable to read value at 0x%llx",position);
        return 0;
    }

    result = *((T*)&bytes);
    if (QSysInfo::ByteOrder == QSysInfo::BigEndian)
        result = qToBigEndian(result);

    return result;
}

qint64 zip_reader::find_next(QFile &file,qint64 position,QByteArray header)
{
    int current_index = 0;
    QByteArray file_data;
    file.seek(position);
    while (!file.atEnd()){
        file_data = file.read(1024);
        for	(int i = 0;i < file_data.length();i++){
            position++;
            if (file_data[i] == header[current_index]){
                current_index++;
                if (current_index == header.length()){
                    return position-header.length();
                }

            } else {
                current_index = 0;
            }
        }
    }
    return -1;
}

void zip_reader::read_file(QByteArray path)
{
    qDebug("%s",path.data());
    m_file = path;
    QFile file(m_file);
    qint64 header_start = 0;

    if (!file.open(QIODevice::ReadOnly)){
        qDebug("Unable to open file %s",path.data());
        isValid = false;
        return;
    }

    isValid = true;

    while ((header_start = find_next(file,header_start,SOCD_HEADER)) >= 0){
        zipped_file zfile;

        zfile.compression_type = get_number<unsigned short>(file,header_start+file_compression_type);

        zfile.compressed_size   = get_number<unsigned>(file,header_start+file_compressed_size);
        zfile.uncompressed_size = get_number<unsigned>(file,header_start+file_size);

        unsigned short filename_len = 0,extra_len = 0,comment_len = 0;
        filename_len = get_number<unsigned short>(file,header_start+file_len_location);
        extra_len    = get_number<unsigned short>(file,header_start+file_extra_len_location);
        comment_len  = get_number<unsigned short>(file,header_start+file_comment_len_locaion);

        unsigned short starting_disk;
        starting_disk = get_number<unsigned short>(file,header_start+file_starting_disk);

        zfile.header_position = get_number<unsigned>(file,header_start+file_location);

        zfile.filename = get_string_at(file,header_start+filename_location,filename_len);

        if (!isValid)
            goto error;

        if (starting_disk > 0){
            qDebug("multi disk zips are not supported");
        } else if (zfile.compression_type != 0){
            qDebug("only zip files that are uncompressed are supported");
        } else {
            m_files.append(zfile);
        }

        //qDebug("SIZE:%u->%u,TYPE:%u,POS:%u\n%s",zfile.compressed_size,zfile.uncompressed_size,zfile.compression_type,zfile.header_position,zfile.filename.data());

        header_start += filename_len+extra_len+comment_len;

    }
    if (m_files.length() == 0){
        isValid = false;
        goto error;
    }
    file.close();
    return;
error:
    file.close();
    qDebug("an error has ocurred while reading file %s",path.data());
}

int zip_reader::file_count()
{
    if (!isValid)
        return 0;
    return m_files.length();
}


QByteArray zip_reader::get_file_data(int i)
{
    if (!isValid || i >= m_files.length())
        return QByteArray();

    QByteArray data;
    zipped_file zfile = m_files.at(i);
    QFile file(m_file);
    if (!file.open(QIODevice::ReadOnly)){
        qDebug("Unable to open file %s",m_file.data());
        isValid = false;
        return QByteArray();
    }
    QByteArray filename;
    QByteArray header = get_string_at(file,zfile.header_position,4);
    if (qstrncmp(header,LF_HEADER,4))
        goto error;

    unsigned short filename_len,extra_len;
    filename_len = get_number<unsigned short>(file,zfile.header_position+lf_filename_len);
    extra_len = get_number<unsigned short>(file,zfile.header_position+lf_extra_len);

    //filename = get_string_at(file,zfile.header_position+lf_filename,filename_len);
    data = get_string_at(file,zfile.header_position+lf_filename+filename_len+extra_len,zfile.compressed_size);
    if (!isValid)
        goto error;
    //qDebug("filenamelen:%d,%s",filename_len,filename.data());


    file.close();
    return data;

error:
    qDebug("Invalid file header");
    isValid = false;
    file.close();
    return QByteArray();
}
