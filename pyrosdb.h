#ifndef PYROSDB_H
#define PYROSDB_H

#include <QThread>
#include <QObject>
#include <QPointer>

#include <pyros.h>


class PyrosTC : public QObject{
    Q_OBJECT

    static PyrosTC *instance;
    PyrosTC();

    PyrosDB *db;
    QThread workerThread;

public:
    ~PyrosTC();
    static PyrosTC* get();

    typedef std::function<void(QVector<PyrosFile*>)> search_cb;
    typedef std::function<void(QVector<PyrosTag*>)> tag_cb;
    typedef std::function<void(int)> import_progress_cb;
    typedef std::function<void(QStringList)> all_tags_cb;
    typedef void(PyrosExtFunc)(PyrosDB*,const char*,const char*);

private:

    enum REQUEST_FLAGS{
        NONE        = 0x00,
        OVERRIDE    = 0x08,
    };

    struct request{
        QPointer<QObject> sender;
        uint flags;
        search_cb s_cb;
        import_progress_cb ip_cb;
        tag_cb t_cb;
        all_tags_cb at_cb;
        bool discard;
    };
    QVector<struct request> requests;

    void push_request(struct request req);

public slots:
    void search_return(QVector<PyrosFile*> files);
    void progress(int prog);
    void tag_return(QVector<PyrosTag*> tags);
    void return_all_tags(QStringList tags);

signals:
    void sig_add_tags(PyrosDB *db,QVector<QByteArray> hashes, QVector<QByteArray>tags);
    void sig_add_tags_to_file(PyrosDB *db,QByteArray hashes, QVector<QByteArray>tags);
    void sig_search(PyrosDB *db,QVector<QByteArray> tags);
    void sig_remove_tags(PyrosDB *db,QVector<QByteArray> hashes, QVector<QByteArray>tags);
    void sig_remove_tags_from_file(PyrosDB *db,QByteArray hashes, QVector<QByteArray>tags);
    void sig_import(PyrosDB *db,QVector<QByteArray> files,bool use_tag_files,QVector<QByteArray> import_tags);
    void sig_get_tags_from_hash(PyrosDB* db,QByteArray hash);
    void sig_delete_file(PyrosDB* db,PyrosFile*file);
    void sig_delete_files(PyrosDB* db,QVector<PyrosFile*>files);
    void sig_ext_func(PyrosDB *db,QVector<QByteArray>,QVector<QByteArray>,PyrosTC::PyrosExtFunc*);
    void sig_close(PyrosDB *db);
    void sig_remove_ext(PyrosDB*,QVector<QByteArray>);
    void sig_get_all_tags(PyrosDB *);

public:
    void add_tags(QVector<QByteArray> hashes, QVector<QByteArray>tags);
    void add_tags(QByteArray hashes, QVector<QByteArray>tags);

    void search(QPointer<QObject> sender,QVector<QByteArray> tags, PyrosTC::search_cb func);

    void remove_tags(QVector<QByteArray>hashes ,QVector<QByteArray>tags);
    void remove_tags(QByteArray hash ,QVector<QByteArray>tags);

    void import(QPointer<QObject> sender,QVector<QByteArray> files, search_cb cb,import_progress_cb prog_cb,bool use_tag_files,QVector<QByteArray> import_tags);

    void get_tags_from_hash(QPointer<QObject> sender,QByteArray hash, PyrosTC::tag_cb func);

    void delete_file(PyrosFile*file);
    void delete_file(QVector<PyrosFile*>files);

    void add_alias(QVector<QByteArray> tag, QVector<QByteArray> alises);
    void add_child(QVector<QByteArray> tag, QVector<QByteArray> children);
    void add_parent(QVector<QByteArray> tag, QVector<QByteArray> parents);
    void get_all_tags(QPointer<QObject>sender,all_tags_cb cb);
    void close_db();

    void remove_ext(QVector<QByteArray> tags);
};

class PyrosWorker : public QObject
{
    Q_OBJECT
private:
    QVector<const char*> QVBA_to_QVc(QVector<QByteArray> &vec);

public slots:
    void add_tags(PyrosDB *db, QVector<QByteArray> hashes,  QVector<QByteArray>tags);
    void add_tags_to_file(PyrosDB *db, QByteArray hashes, QVector<QByteArray>tags);

    void search(PyrosDB *db,QVector<QByteArray> tags);
    void remove_tags(PyrosDB *db,QVector<QByteArray> hashes, QVector<QByteArray>tags);
    void remove_tags_from_file(PyrosDB *db,QByteArray hash, QVector<QByteArray>tags);
    void import(PyrosDB *db,QVector<QByteArray> files,bool use_tag_files,QVector<QByteArray> import_tags);
    void get_tags_from_hash(PyrosDB* db,QByteArray hash);
    void delete_file(PyrosDB* db,PyrosFile*);
    void delete_files(PyrosDB* db,QVector<PyrosFile*>files);
    void ext_func(PyrosDB *db,QVector<QByteArray>,QVector<QByteArray>,PyrosTC::PyrosExtFunc*);
    void close_db(PyrosDB *db);
    void remove_ext(PyrosDB *db,QVector<QByteArray>tags);
    void get_all_tags(PyrosDB *db);

signals:
    void search_return(QVector<PyrosFile*>);
    void report_progress(int);
    void tag_return(QVector<PyrosTag*>);
    void return_all_tags(QStringList);
};

Q_DECLARE_METATYPE(QVector<QByteArray>)
Q_DECLARE_METATYPE(QVector<PyrosFile*>)
Q_DECLARE_METATYPE(QVector<PyrosTag*>)
Q_DECLARE_METATYPE(PyrosTC::PyrosExtFunc*)


#endif // PYROSDB_H
