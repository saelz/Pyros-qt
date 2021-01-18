#include "pyrosdb.h"
#include "filemodel.h"

#include <QtDebug>
#include <QSettings>

PyrosTC *PyrosTC::instance = 0;

QVector<const char*> PyrosWorker::QVBA_to_QVc(QVector<QByteArray> &vec){
     QVector<const char*> cvec;

    foreach(QByteArray item,vec)
        cvec.push_back(item);

    return cvec;
}

void PyrosWorker::add_tags(PyrosDB *db,QVector<QByteArray> hashes, QVector<QByteArray>tags){
    QVector<const char*> ctags  = QVBA_to_QVc(tags);

    foreach(QByteArray hash,hashes)
        Pyros_Add_Tag(db,hash,(char**)ctags.data(),ctags.size());

    Pyros_Commit(db);
}

void PyrosWorker::add_tags_to_file(PyrosDB *db,QByteArray hash, QVector<QByteArray>tags){
    QVector<const char*> ctags  = QVBA_to_QVc(tags);

    Pyros_Add_Tag(db,hash,(char**)ctags.data(),ctags.size());
    Pyros_Commit(db);
}

void PyrosWorker::search(PyrosDB *db, QVector<QByteArray> tags){
    QVector<const char*> ctags  = QVBA_to_QVc(tags);
    PyrosList *files;
    QVector<PyrosFile*> vec_files;

    files = Pyros_Search(db,(char**)ctags.data(),ctags.size());

    for(size_t i = 0; i < files->length;i++)
        vec_files.push_back((PyrosFile*)files->list[i]);

    Pyros_List_Free(files,nullptr);
    emit search_return(vec_files);
}

void PyrosWorker::remove_tags(PyrosDB *db,QVector<QByteArray> hashes, QVector<QByteArray>tags){

    foreach(QByteArray hash,hashes)
        foreach(QByteArray tag,tags)
                Pyros_Remove_Tag_From_Hash(db,hash,tag);

    Pyros_Commit(db);
}

void PyrosWorker::remove_tags_from_file(PyrosDB *db,QByteArray hash, QVector<QByteArray>tags){

    foreach(QByteArray tag,tags)
        Pyros_Remove_Tag_From_Hash(db,hash,tag);

    Pyros_Commit(db);
}

void PyrosWorker::import(PyrosDB *db, QVector<QByteArray> files,bool use_tag_files,QVector<QByteArray> import_tags){
    QVector<const char*> ctags  = QVBA_to_QVc(files);
    QVector<const char*> cimport_tags  = QVBA_to_QVc(import_tags);
    PyrosList *hashes;
    QVector<PyrosFile*> return_files;

    Pyros_Add_Full_Callback add_callback = [](char*origin_hash,char* filepath,size_t prog,void *voidptr)
    {
        Q_UNUSED(origin_hash)
        Q_UNUSED(filepath)
        Q_UNUSED(prog)
        PyrosWorker *self = (PyrosWorker*)voidptr;
        emit self->report_progress(prog);

    };

    hashes = Pyros_Add_Full(db,
                            (char**)ctags.data(),ctags.length(),
                            (char**)cimport_tags.data(),cimport_tags.length(),
                            use_tag_files,true,
                            add_callback,this);
    Pyros_Commit(db);

    for(size_t i = 0; i < hashes->length;i++){
        qDebug("H%s\n",(char*)hashes->list[i]);
        PyrosFile *pFile = Pyros_Get_File_From_Hash(db,(char*)hashes->list[i]);
        if (pFile != nullptr)
            return_files.push_back(pFile);
    }

    Pyros_List_Free(hashes,free);
    emit search_return(return_files);
}

void PyrosWorker::get_tags_from_hash(PyrosDB *db, QByteArray hash){
    QVector<PyrosTag*> vec;
    PyrosList *tags;

    tags = Pyros_Get_Tags_From_Hash(db,hash.data());

    for (size_t i = 0; i < tags->length; i++)
            vec.push_back((PyrosTag*)tags->list[i]);

    Pyros_List_Free(tags,nullptr);
    emit tag_return(vec);
}

void PyrosWorker::delete_file(PyrosDB *db, PyrosFile*pFile){
    Pyros_Remove_File(db,pFile);
    FileModel::delete_thumbnail(pFile->hash);
    Pyros_Close_File(pFile);
    Pyros_Commit(db);
}

void PyrosWorker::delete_files(PyrosDB *db, QVector<PyrosFile*>files){
    foreach(PyrosFile*pFile,files){
        Pyros_Remove_File(db,pFile);
        FileModel::delete_thumbnail(pFile->hash);
        Pyros_Close_File(pFile);
    }
    Pyros_Commit(db);
}

void PyrosWorker::ext_func(PyrosDB* db,QVector<QByteArray> tags, QVector<QByteArray> sub_tags,PyrosTC::PyrosExtFunc ExtFunc){
    foreach(QByteArray tag,tags){
        foreach(QByteArray sub_tag,sub_tags){
            ExtFunc(db,tag,sub_tag);
        }
    }

    Pyros_Commit(db);
}

void PyrosWorker::close_db(PyrosDB *db){
    Pyros_Close_Database(db);
}

void PyrosWorker::remove_ext(PyrosDB*db,QVector<QByteArray> tags){
    QVector<const char*> ctags  = QVBA_to_QVc(tags);
    for(int i = 1; i < ctags.count();i+=2)
        Pyros_Remove_Tag_Relationship(db,ctags.at(i-1),ctags.at(i));
    Pyros_Commit(db);
}

void PyrosWorker::get_all_tags(PyrosDB *db){
    PyrosList *all_tags = Pyros_Get_All_Tags(db);
    QStringList tags;

    for (size_t i = 0;i < all_tags->length; i++)
        tags << (char*)all_tags->list[i];

    Pyros_List_Free(all_tags,free);

    emit return_all_tags(tags);
}

PyrosTC::~PyrosTC(){
    workerThread.quit();
    workerThread.wait();

    if (db != nullptr)
        Pyros_Close_Database(db);
}

PyrosTC::PyrosTC()
{
    db = nullptr;

    qRegisterMetaType<QVector<QByteArray>>("QVecByteArray");
    qRegisterMetaType<QVector<PyrosFile*>>("QVecPyrosFileptr");
    qRegisterMetaType<QVector<PyrosTag*>>("QVecPyrosTagptr");
    qRegisterMetaType<PyrosTC::PyrosExtFunc*>("PyrosExtFunc");

    PyrosWorker *worker = new PyrosWorker;
    worker->moveToThread(&workerThread);
    connect(&workerThread, &QThread::finished, worker, &PyrosWorker::deleteLater);

    connect(this,&PyrosTC::sig_add_tags,
        worker,&PyrosWorker::add_tags);
    connect(this,&PyrosTC::sig_add_tags_to_file,
        worker,&PyrosWorker::add_tags_to_file);

    connect(this,&PyrosTC::sig_search,
        worker,&PyrosWorker::search);

    connect(worker,&PyrosWorker::search_return,
        this,&PyrosTC::search_return);


    connect(this,&PyrosTC::sig_remove_tags,
        worker,&PyrosWorker::remove_tags);
    connect(this,&PyrosTC::sig_remove_tags_from_file,
        worker,&PyrosWorker::remove_tags_from_file);

    connect(this,&PyrosTC::sig_import,
        worker,&PyrosWorker::import);
    connect(worker,&PyrosWorker::report_progress,
            this,&PyrosTC::progress);

    connect(this,&PyrosTC::sig_get_tags_from_hash,
        worker,&PyrosWorker::get_tags_from_hash);
    connect(worker,&PyrosWorker::tag_return,
        this,&PyrosTC::tag_return);

    connect(this,&PyrosTC::sig_delete_file,
        worker,&PyrosWorker::delete_file);
    connect(this,&PyrosTC::sig_delete_files,
        worker,&PyrosWorker::delete_files);

    connect(this,&PyrosTC::sig_ext_func,
        worker,&PyrosWorker::ext_func);

    connect(this,&PyrosTC::sig_close,
        worker,&PyrosWorker::close_db);

    connect(this,&PyrosTC::sig_remove_ext,
        worker,&PyrosWorker::remove_ext);

    connect(this,&PyrosTC::sig_get_all_tags,
        worker,&PyrosWorker::get_all_tags);
    connect(worker,&PyrosWorker::return_all_tags,
        this,&PyrosTC::return_all_tags);

    workerThread.start();



}

QByteArray PyrosTC::escape_glob_characters(QString tag){
    QString escaped_tag;
    foreach (QChar c ,tag){
        if (c == '*' || c == '?' || c == '[' || c == ']'){
            escaped_tag.append('[');
            escaped_tag.append(c);
            escaped_tag.append(']');

        } else {
            escaped_tag.append(c);
        }
    }
    return escaped_tag.toUtf8();
}

QByteArray PyrosTC::db_path()
{
    if (db == nullptr)
        return QByteArray();
    return db->path;
}

PyrosTC* PyrosTC::get()
{
    if (!instance){
        instance = new PyrosTC;
    }
    if (instance->db == nullptr){
        QSettings settings;
        QByteArray path = settings.value("db","").toByteArray();
        if (Pyros_Database_Exists(path))
            instance->db = Pyros_Open_Database(path);
    }
    return instance;

}


void PyrosTC::push_request(struct request req){
    if (req.flags & OVERRIDE){
        for(int i = 0; i < requests.length(); i++){
            struct request *r = &requests[i];
            if (!r->discard  && r->sender == req.sender &&
                    r->flags == req.flags){
                r->discard = true;
                break;
            }
        }
    }

    requests.push_back(req);
}

void PyrosTC::search_return(QVector<PyrosFile*> files){
    struct request req = requests.front();

    if (req.discard || req.sender.isNull()){
        foreach(PyrosFile *pFile, files)
            Pyros_Close_File(pFile);
    } else {
        req.s_cb(files);
    }

    requests.pop_front();
}

void PyrosTC::tag_return(QVector<PyrosTag*> tags){
    struct request req = requests.front();

    if (req.discard || req.sender.isNull()){
        foreach(PyrosTag *tag, tags)
            Pyros_Free_Tag(tag);
    } else {
        req.t_cb(tags);
    }

    requests.pop_front();
}

void PyrosTC::return_all_tags(QStringList tags){
    struct request req = requests.front();

    if (!req.discard && !req.sender.isNull()){
        req.at_cb(tags);
    }

    requests.pop_front();
}

void PyrosTC::progress(int prog){
    struct request req = requests.at(0);
    if (!req.discard && !req.sender.isNull())
        req.ip_cb(prog);
}

void PyrosTC::add_tags(QVector<QByteArray> hashes, QVector<QByteArray> tags){
    if (db == nullptr) return;
    emit sig_add_tags(db,hashes,tags);
}
void PyrosTC::add_tags(QByteArray hash, QVector<QByteArray> tags){
    if (db == nullptr) return;
    emit sig_add_tags_to_file(db,hash,tags);
}

void PyrosTC::search(QPointer<QObject>sender,QVector<QByteArray> tags, search_cb func){
    struct request req = {sender,OVERRIDE,func,nullptr,nullptr,nullptr,false};
    if (db == nullptr) return;
    push_request(req);
    emit sig_search(db,tags);
}


void PyrosTC::remove_tags(QVector<QByteArray> hashes, QVector<QByteArray> tags){
    if (db == nullptr) return;
    emit sig_remove_tags(db,hashes,tags);
}

void PyrosTC::remove_tags(QByteArray hash, QVector<QByteArray> tags){
    if (db == nullptr) return;
    emit sig_remove_tags_from_file(db,hash,tags);
}

void PyrosTC::import(QPointer<QObject>sender,QVector<QByteArray> files, search_cb func,import_progress_cb ib,bool use_tag_files,QVector<QByteArray> import_tags){
    struct request req = {sender,NONE,func,ib,nullptr,nullptr,false};
    if (db == nullptr) return;
    push_request(req);

    emit sig_import(db,files,use_tag_files,import_tags);
}

void PyrosTC::get_tags_from_hash(QPointer<QObject>sender, QByteArray hash, PyrosTC::tag_cb func) {
    struct request req = {sender,OVERRIDE,nullptr,nullptr,func,nullptr,false};
    if (db == nullptr) return;
    push_request(req);

    emit sig_get_tags_from_hash(db,hash);
}

void PyrosTC::delete_file(PyrosFile *file){
    if (db == nullptr) return;
    emit sig_delete_file(db,file);
}

void PyrosTC::delete_file(QVector<PyrosFile*> files){
    if (db == nullptr) return;
    emit sig_delete_files(db,files);
}

void PyrosTC::add_alias(QVector<QByteArray> tag, QVector<QByteArray> alises){
    if (db == nullptr) return;
    emit sig_ext_func(db,tag,alises,Pyros_Add_Alias);
}

void PyrosTC::add_parent(QVector<QByteArray> tag, QVector<QByteArray> parents){
    if (db == nullptr) return;
    emit sig_ext_func(db,tag,parents,Pyros_Add_Parent);
}

void PyrosTC::add_child(QVector<QByteArray> tag, QVector<QByteArray> children){
    if (db == nullptr) return;
    emit sig_ext_func(db,tag,children,Pyros_Add_Child);
}

void PyrosTC::close_db(){
    if (db == nullptr) return;
    emit sig_close(db);
    db = nullptr;
}

void PyrosTC::remove_ext(QVector<QByteArray> tags){
    if (db == nullptr) return;
    emit sig_remove_ext(db,tags);
}

void PyrosTC::get_all_tags(QPointer<QObject>sender,all_tags_cb cb){
    struct request req = {sender,OVERRIDE,nullptr,nullptr,nullptr,cb,false};
    if (db == nullptr) return;
    push_request(req);
    emit sig_get_all_tags(db);
}
