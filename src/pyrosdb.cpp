#include "pyrosdb.h"
#include "thumbnailer.h"

#include <QSettings>
#include <QDateTime>
#include <stddef.h>

PyrosTC *PyrosTC::instance = 0;

QVector<const char*> PyrosWorker::QVBA_to_QVc(QVector<QByteArray> &vec)
{
    QVector<const char*> cvec;

    foreach(QByteArray item,vec)
        cvec.push_back(item);

    return cvec;
}

void PyrosWorker::show_error(PyrosDB *db)
{
    QString err(Pyros_Get_Error_Message(db));
    emit error_occurred(err);
    Pyros_Clear_Error(db);
    Pyros_Rollback(db);
    emit request_finished();
}

void PyrosWorker::add_tags(PyrosDB *db,QVector<QByteArray> hashes, QVector<QByteArray>tags)
{
    QVector<const char*> ctags  = QVBA_to_QVc(tags);

    foreach(QByteArray hash,hashes){
        if (Pyros_Add_Tag(db,hash,(char**)ctags.data(),ctags.size()) != PYROS_OK)
            return show_error(db);
    }

    if (Pyros_Commit(db) != PYROS_OK)
        return show_error(db);
    emit request_finished();
}

void PyrosWorker::add_tags_to_file(PyrosDB *db,QByteArray hash, QVector<QByteArray>tags)
{
    QVector<const char*> ctags  = QVBA_to_QVc(tags);

    if (Pyros_Add_Tag(db,hash,(char**)ctags.data(),ctags.size()) != PYROS_OK)
        return show_error(db);

    if (Pyros_Commit(db) != PYROS_OK)
        return show_error(db);
    emit request_finished();
}

void PyrosWorker::search(PyrosDB *db, QVector<QByteArray> tags)
{
    QVector<const char*> ctags  = QVBA_to_QVc(tags);
    PyrosList *files;
    QVector<PyrosFile*> vec_files;

    files = Pyros_Search(db,(char**)ctags.data(),ctags.size());

    if (files == NULL){
        emit search_return(vec_files);
        return show_error(db);
    }

    for(size_t i = 0; i < files->length;i++)
        vec_files.push_back((PyrosFile*)files->list[i]);

    Pyros_List_Free(files,nullptr);
    emit search_return(vec_files);
    emit request_finished();
}

void PyrosWorker::remove_tags(PyrosDB *db,QVector<QByteArray> hashes, QVector<QByteArray>tags)
{

    foreach(QByteArray hash,hashes)
        foreach(QByteArray tag,tags)
                Pyros_Remove_Tag_From_Hash(db,hash,tag);

    if (Pyros_Commit(db) != PYROS_OK)
        return show_error(db);
    emit request_finished();
}

void PyrosWorker::remove_tags_from_file(PyrosDB *db,QByteArray hash, QVector<QByteArray>tags)
{

    foreach(QByteArray tag,tags)
        Pyros_Remove_Tag_From_Hash(db,hash,tag);

    if (Pyros_Commit(db) != PYROS_OK)
        return show_error(db);
    emit request_finished();
}

void PyrosWorker::import(PyrosDB *db, QVector<QByteArray> files,bool use_tag_files,QVector<QByteArray> import_tags)
{
    QVector<const char*> ctags  = QVBA_to_QVc(files);
    QVector<const char*> cimport_tags  = QVBA_to_QVc(import_tags);
    PyrosList *hashes;
    QVector<PyrosFile*> return_files;

    Pyros_Add_Full_Callback add_callback = [](const char*origin_hash,const char* filepath,size_t prog,void *voidptr)
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
    if (hashes == NULL){
        emit search_return(return_files);
        return show_error(db);
    }

    if (Pyros_Commit(db) != PYROS_OK)
        return show_error(db);

    for(size_t i = 0; i < hashes->length;i++){
        PyrosFile *pFile = Pyros_Get_File_From_Hash(db,(char*)hashes->list[i]);

        if (Pyros_Get_Error_Type(db) != PYROS_OK)
            return show_error(db);

        if (pFile != nullptr)
            return_files.push_back(pFile);
    }

    Pyros_List_Free(hashes,free);
    emit search_return(return_files);
    emit request_finished();
}

void PyrosWorker::get_tags_from_hash(PyrosDB *db, QByteArray hash)
{
    QVector<PyrosTag*> vec;
    PyrosList *tags;

    tags = Pyros_Get_Tags_From_Hash(db,hash.data());

    if (tags == NULL)
            return show_error(db);

    for (size_t i = 0; i < tags->length; i++)
            vec.push_back((PyrosTag*)tags->list[i]);

    Pyros_List_Free(tags,nullptr);
    emit tag_return(vec);
    emit request_finished();
}

void PyrosWorker::get_related_tags(PyrosDB *db, QVector<QByteArray> tags,uint relation_type)
{
    QVector<PyrosList*> related_tags;
    QVector<QByteArray> unfound_tags;
    foreach(QByteArray tag,tags){
        PyrosList *t;
        if (!tag.compare("*")){
            unfound_tags.append(tag);
        } else{
            t = Pyros_Get_Related_Tags(db,tag.data(),relation_type);

            if (Pyros_Get_Error_Type(db) != PYROS_OK){
                emit related_tag_return(related_tags,unfound_tags);
                return show_error(db);
            }

            if (t == nullptr || t->length == 0)
                unfound_tags.append(tag);
            else
                related_tags.append(t);

            if (t != nullptr && t->length == 0)
                Pyros_List_Free(t,nullptr);
        }
    }

    emit related_tag_return(related_tags,unfound_tags);
    emit request_finished();

}

void PyrosWorker::delete_file(PyrosDB *db, PyrosFile*pFile)
{
    if (Pyros_Remove_File(db,pFile) != PYROS_OK)
        return show_error(db);

    Thumbnailer::delete_thumbnail(pFile->hash);
    Pyros_Free_File(pFile);
    if (Pyros_Commit(db) != PYROS_OK)
        return show_error(db);
    emit request_finished();
}

void PyrosWorker::delete_files(PyrosDB *db, QVector<PyrosFile*>files)
{
    bool errored = false;
    foreach(PyrosFile*pFile,files){
        if (!errored){
            if (Pyros_Remove_File(db,pFile) != PYROS_OK)
                errored = true;

            Thumbnailer::delete_thumbnail(pFile->hash);
        }
        Pyros_Free_File(pFile);
    }

    if (errored || Pyros_Commit(db) != PYROS_OK)
        return show_error(db);
    emit request_finished();
}

void PyrosWorker::tag_relation_func(PyrosDB* db,QVector<QByteArray> tags, QVector<QByteArray> sub_tags,PyrosTC::PyrosExtFunc ExtFunc)
{
    foreach(QByteArray tag,tags){
        foreach(QByteArray sub_tag,sub_tags){
            if (ExtFunc(db,tag,sub_tag) != PYROS_OK)
                return show_error(db);
        }
    }

    if (Pyros_Commit(db) != PYROS_OK)
        return show_error(db);
    emit request_finished();
}

void PyrosWorker::close_db(PyrosDB *db)
{
    if (Pyros_Close_Database(db) != PYROS_OK)
        return show_error(db);
    emit request_finished();
}

void PyrosWorker::remove_relationship(PyrosDB*db,QVector<QByteArray> tags)
{
    QVector<const char*> ctags  = QVBA_to_QVc(tags);
    for(int i = 1; i < ctags.count();i+=2)
        if (Pyros_Remove_Tag_Relationship(db,ctags.at(i-1),ctags.at(i)) != PYROS_OK)
            return show_error(db);

    if (Pyros_Commit(db) != PYROS_OK)
        return show_error(db);

    emit request_finished();
}

void PyrosWorker::get_all_tags(PyrosDB *db)
{
    PyrosList *all_tags = Pyros_Get_All_Tags(db);
    QStringList tags;

    if (all_tags == NULL){
        emit return_all_tags(tags);
        return show_error(db);
    }

    for (size_t i = 0;i < all_tags->length; i++)
        tags << (char*)all_tags->list[i];

    Pyros_List_Free(all_tags,free);

    emit return_all_tags(tags);
    emit request_finished();
}

void PyrosWorker::merge_files(PyrosDB *db,QByteArray superior_file,QVector<QByteArray> duplicates)
{
    foreach(QByteArray duplicate,duplicates){
        Thumbnailer::delete_thumbnail(duplicate);
        if (Pyros_Merge_Hashes(db,superior_file,duplicate,true) != PYROS_OK)
            return show_error(db);
    }

    if (Pyros_Commit(db) != PYROS_OK)
        return show_error(db);

    emit request_finished();

}

void PyrosWorker::vacuum_database(PyrosDB *db)
{
    if (Pyros_Vacuum_Database(db) != PYROS_OK)
        return show_error(db);
    emit request_finished();
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
    qRegisterMetaType<QVector<PyrosList*>>("QVecPyrosListptr");
    qRegisterMetaType<PyrosTC::PyrosExtFunc*>("PyrosExtFunc");

    PyrosWorker *worker = new PyrosWorker;
    worker->moveToThread(&workerThread);
    connect(&workerThread, &QThread::finished, worker, &PyrosWorker::deleteLater);
    connect(worker,&PyrosWorker::error_occurred,
        this,&PyrosTC::error_occurred);


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

    connect(this,&PyrosTC::sig_get_related_tags,
        worker,&PyrosWorker::get_related_tags);
    connect(worker,&PyrosWorker::related_tag_return,
        this,&PyrosTC::related_tag_return);

    connect(this,&PyrosTC::sig_delete_file,
        worker,&PyrosWorker::delete_file);
    connect(this,&PyrosTC::sig_delete_files,
        worker,&PyrosWorker::delete_files);

    connect(this,&PyrosTC::sig_tag_relation_func,
        worker,&PyrosWorker::tag_relation_func);

    connect(this,&PyrosTC::sig_close,
        worker,&PyrosWorker::close_db);

    connect(this,&PyrosTC::sig_remove_relationship,
        worker,&PyrosWorker::remove_relationship);


    connect(this,&PyrosTC::sig_merge_files,
        worker,&PyrosWorker::merge_files);

    connect(this,&PyrosTC::sig_get_all_tags,
        worker,&PyrosWorker::get_all_tags);
    connect(worker,&PyrosWorker::return_all_tags,
        this,&PyrosTC::return_all_tags);

    connect(this,&PyrosTC::sig_vacuum_database,
        worker,&PyrosWorker::vacuum_database);

    connect(worker,&PyrosWorker::request_finished,
        this,&PyrosTC::request_finsished);

    workerThread.start();

}

QByteArray PyrosTC::db_path()
{
    if (db == nullptr)
        return QByteArray();
    return Pyros_Get_Database_Path(db);
}

PyrosTC* PyrosTC::get()
{
    if (!instance){
        instance = new PyrosTC;
    }
    if (instance->db == nullptr){
        QSettings settings;
        QByteArray path = settings.value("db","").toByteArray();
        if (Pyros_Database_Exists(path)){
            instance->db = Pyros_Alloc_Database(path.data());
            if (Pyros_Open_Database(instance->db) != PYROS_OK){
                emit instance->error_occurred(Pyros_Get_Error_Message(instance->db));
                Pyros_Close_Database(instance->db);
                delete instance;
                return NULL;
            }
        }
    }
    return instance;

}


void PyrosTC::push_request(Request req)
{
    if (req.flags & OVERRIDE){
        for(int i = 0; i < requests.length(); i++){
            Request *r = &requests[i];
            if (!r->discard  && r->sender == req.sender &&
                    r->flags == req.flags){
                r->discard = true;
                break;
            }
        }
    }

    requests.push_back(req);
    process_requests();
}

void PyrosTC::process_requests()
{

    if (requests.empty())
        return;

    Request *req = &requests.front();

    if (req->discard && !req->active){
        requests.pop_front();
        process_requests();
    } else if (!req->active){
        req->active = true;
        req->execute_cmd();
    }
}

void PyrosTC::request_finsished()
{
    requests.pop_front();
    process_requests();
}


void PyrosTC::search_return(QVector<PyrosFile*> files)
{
    Request req = requests.front();

    if (req.discard || req.sender.isNull()){
        foreach(PyrosFile *pFile, files)
            Pyros_Free_File(pFile);
    } else {
        req.s_cb(files);
    }

}

void PyrosTC::tag_return(QVector<PyrosTag*> tags)
{
    Request req = requests.front();

    if (req.discard || req.sender.isNull()){
        foreach(PyrosTag *tag, tags)
            Pyros_Free_Tag(tag);
    } else {
        req.t_cb(tags);
    }

}

void PyrosTC::related_tag_return(QVector<PyrosList*> related_tags,QVector<QByteArray> unfound_tags)
{
    Request req = requests.front();

    if (req.discard || req.sender.isNull()){
        foreach(PyrosList *tags, related_tags)
            Pyros_List_Free(tags,(Pyros_Free_Callback)Pyros_Free_Tag);
    } else {
        req.r_cb(related_tags,unfound_tags);
    }

}

void PyrosTC::return_all_tags(QStringList tags)
{
    known_tags = tags;
}

void PyrosTC::progress(int prog)
{
    Request req = requests.at(0);
    if (!req.discard && !req.sender.isNull())
        req.ip_cb(prog);
}

void PyrosTC::add_tags(QVector<QByteArray> hashes, QVector<QByteArray> tags)
{
    if (db == nullptr) return;

    foreach(QByteArray tag, tags)
        if (!known_tags.contains(tag))
            known_tags.append(tag);

    emit tag_added(hashes,tags);

    related_cb r_cb = [&](QVector<PyrosList*> related_tags,QVector<QByteArray> unfound_tags){
        emit tags_added_with_related(related_tags,unfound_tags);
        foreach(PyrosList *tags,related_tags)
            Pyros_List_Free(tags,(Pyros_Free_Callback)Pyros_Free_Tag);
    };

    push_request(Request(this,NONE,r_cb,[=](){
        emit sig_get_related_tags(db,tags,PYROS_FILE_RELATIONSHIP);
    }));

    push_request(Request(NONE,[=](){
        emit sig_add_tags(db,hashes,tags);
    }));

}

void PyrosTC::get_related_tags(QPointer<QObject> sender,QVector<QByteArray> tags,related_cb cb,uint relation_type)
{
    if (db == nullptr) return;

    push_request(Request(sender,NONE,cb,[=](){
        emit sig_get_related_tags(db,tags,relation_type);
    }));

}

void PyrosTC::add_tags(QByteArray hash, QVector<QByteArray> tags)
{
    if (db == nullptr) return;
    foreach(QByteArray tag, tags)
        if (!known_tags.contains(tag))
            known_tags.append(tag);

    emit tag_added(QVector<QByteArray>{hash},tags);
    related_cb r_cb = [&](QVector<PyrosList*> related_tags,QVector<QByteArray> unfound_tags){
        emit tags_added_with_related(related_tags,unfound_tags);
        foreach(PyrosList *tags,related_tags)
            Pyros_List_Free(tags,(Pyros_Free_Callback)Pyros_Free_Tag);
    };

    push_request(Request(this,NONE,r_cb,[=](){
        emit sig_get_related_tags(db,tags,PYROS_FILE_RELATIONSHIP);
    }));

    push_request(Request(NONE,[=](){
        emit sig_add_tags_to_file(db,hash,tags);
    }));

}


void PyrosTC::search(QPointer<QObject>sender,QVector<QByteArray> tags, search_cb func)
{
    if (db == nullptr) return;
    push_request(Request(sender,OVERRIDE,func,nullptr,[=](){
        emit sig_search(db,tags);
    }));
}


void PyrosTC::remove_tags(QVector<QByteArray> hashes, QVector<QByteArray> tags)
{
    if (db == nullptr) return;
    push_request(Request(NONE,[=](){
        emit sig_remove_tags(db,hashes,tags);
    }));

    emit tag_removed(hashes,tags);
}

void PyrosTC::remove_tags(QByteArray hash, QVector<QByteArray> tags)
{
    if (db == nullptr) return;
    push_request(Request(NONE,[=](){
        emit sig_remove_tags_from_file(db,hash,tags);
    }));

    emit tag_removed(QVector<QByteArray>{hash},tags);
}

void PyrosTC::import(QPointer<QObject>sender,QVector<QByteArray> files, search_cb func,import_progress_cb ib,bool use_tag_files,QVector<QByteArray> import_tags)
{
    if (db == nullptr) return;

    push_request(Request(sender,OVERRIDE,func,ib,[=](){
        emit sig_import(db,files,use_tag_files,import_tags);
    }));
}

void PyrosTC::get_tags_from_hash(QPointer<QObject>sender, QByteArray hash, PyrosTC::tag_cb func)
{
    if (db == nullptr) return;

    push_request(Request(sender,OVERRIDE,func,[=](){
        emit sig_get_tags_from_hash(db,hash);
    }));
}

void PyrosTC::delete_file(PyrosFile *file)
{
    if (db == nullptr) return;
    push_request(Request(NONE,[=](){
        emit sig_delete_file(db,file);
    }));

    emit file_removed(QVector<QByteArray>{file->hash});
}

void PyrosTC::delete_file(QVector<PyrosFile*> files)
{
    if (db == nullptr) return;
    push_request(Request(NONE,[=](){
        emit sig_delete_files(db,files);
    }));

    QVector<QByteArray> hashes;
    foreach(PyrosFile *file,files)
        hashes.append(file->hash);

    emit file_removed(hashes);
}

void PyrosTC::add_alias(QVector<QByteArray> tags, QVector<QByteArray> aliases)
{
    if (db == nullptr) return;
    push_request(Request(NONE,[=](){
        emit sig_tag_relation_func(db,tags,aliases,Pyros_Add_Alias);
    }));

    emit tag_relationship_added(tags,aliases,PYROS_ALIAS);
}

void PyrosTC::add_parent(QVector<QByteArray> tags, QVector<QByteArray> parents)
{
    if (db == nullptr) return;
    push_request(Request(NONE,[=](){
        emit sig_tag_relation_func(db,tags,parents,Pyros_Add_Parent);
    }));

    emit tag_relationship_added(tags,parents,PYROS_PARENT);
}

void PyrosTC::add_child(QVector<QByteArray> tags, QVector<QByteArray> children)
{
    if (db == nullptr) return;
    push_request(Request(NONE,[=](){
        emit sig_tag_relation_func(db,tags,children,Pyros_Add_Child);
    }));

    emit tag_relationship_added(tags,children,PYROS_CHILD);
}

void PyrosTC::close_db()
{
    if (db == nullptr) return;
    push_request(Request(NONE,[=](){
        emit sig_close(db);
        db = nullptr;
    }));
}

void PyrosTC::remove_relationship(QVector<QByteArray> tags)
{
    if (db == nullptr) return;
    push_request(Request(NONE,[=](){
        emit sig_remove_relationship(db,tags);
    }));


    emit tag_relationship_removed(tags);
}

void PyrosTC::get_all_tags(QPointer<QObject>sender,all_tags_cb cb)
{
    static qint64 last_check = 0;
    if (db == nullptr) return;

    cb(&known_tags);

    if (QDateTime::currentSecsSinceEpoch()-last_check >= 1800 /*30 min*/){
        last_check = QDateTime::currentSecsSinceEpoch();

        push_request(Request(sender,OVERRIDE,cb,[=](){
            emit sig_get_all_tags(db);
        }));
    }
}

void PyrosTC::merge_files(QByteArray superior_file,QVector<QByteArray> duplicates)
{
    if (db == nullptr) return;
    push_request(Request(NONE,[=](){
        emit sig_merge_files(db,superior_file,duplicates);
    }));

    emit file_removed(duplicates);
}

void PyrosTC::vacuum_database()
{
    if (db == nullptr) return;
    push_request(Request(NONE,[=](){
        emit sig_vacuum_database(db);
    }));
}

PyrosTC::Request::Request(uint flags,std::function<void()> execute_cmd):
    flags(flags),execute_cmd(execute_cmd){}

PyrosTC::Request::Request(QPointer<QObject> sender,uint flags,search_cb s_cb,import_progress_cb ip_cb,std::function<void()> execute_cmd):
    sender(sender),flags(flags),s_cb(s_cb),ip_cb(ip_cb),execute_cmd(execute_cmd){}

PyrosTC::Request::Request(QPointer<QObject> sender,uint flags,tag_cb cb,std::function<void()> execute_cmd):
    sender(sender),flags(flags),t_cb(cb),execute_cmd(execute_cmd){}

PyrosTC::Request::Request(QPointer<QObject> sender,uint flags,all_tags_cb cb,std::function<void()> execute_cmd):
    sender(sender),flags(flags),at_cb(cb),execute_cmd(execute_cmd){}

PyrosTC::Request::Request(QPointer<QObject> sender,uint flags,related_cb cb,std::function<void()> execute_cmd):
    sender(sender),flags(flags),r_cb(cb),execute_cmd(execute_cmd){}

