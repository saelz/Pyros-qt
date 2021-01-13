
#include "fileview.h"
#include "pyrosqt.h"
#include "pyrosdb.h"
#include "configtab.h"

#include <QWindow>
#include <QStatusBar>
#include <QScrollBar>
#include <QMenu>
#include <QClipboard>
#include <QGuiApplication>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QColor>
#include <QResizeEvent>
#include <QPixmap>

#include <iostream>

using ct = configtab;

class FileDelegate : public QStyledItemDelegate {
  public:
    FileModel *model;
    FileDelegate(FileModel *model,QObject *parent )
        : QStyledItemDelegate(parent),model(model) {}

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
                const QModelIndex &index) const {
      QStyledItemDelegate::paint(painter, option, index);
      PyrosFile *pFile = model->file(index);

      QSettings settings;
      if (pFile == nullptr)
          return;
      settings.beginGroup("filecolor");
      QStringList colored_tags = settings.allKeys();
      QString text = pFile->mime;
      QColor color;
      foreach(QString colored_prefix,colored_tags){
          if (text.startsWith(colored_prefix,Qt::CaseInsensitive)){
              color = settings.value(colored_prefix).value<QColor>();
          }
      }
      if (!color.isValid())
          return;
      settings.endGroup();
      painter->setPen(color);

      QPixmap k = model->data(index,Qt::DecorationRole).value<QPixmap>();
      QRect rect = option.rect;

      int vspacing = rect.height()-k.height();
      if (vspacing != 0)
          rect.setTop(rect.top()+vspacing/2);
      rect.setLeft(rect.left()+2);
      rect.setWidth(k.width());
      rect.setHeight(k.height());
      painter->drawRect(rect);
    }

};

FileView::FileView(QWidget *parent) :
    QTableView(parent)
{
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    verticalScrollBar()->setSingleStep(45);
    file_model = new FileModel;
    setModel(file_model);

    contextMenu = new QMenu(this);
    setContextMenuPolicy(Qt::CustomContextMenu);

    contextMenu->addAction("Copy file path",      this,&FileView::copy_path);
    contextMenu->addAction("Hide file",           this,&FileView::hide_file);
    contextMenu->addAction("Regenerate thumbnail",this,&FileView::regenerate_thumbnail);
    contextMenu->addAction("Delete file",         this,&FileView::remove_file);

    connect(this, &FileView::customContextMenuRequested, this, &FileView::onCustomContextMenu);
    connect(this,&FileView::new_files,this, &FileView::get_visible);
    connect(verticalScrollBar(), &QScrollBar::valueChanged, this, &FileView::launch_timer);
    connect(&thumbtimer, &QTimer::timeout, this, &FileView::get_visible);

    fd = new FileDelegate(file_model,this);
    setItemDelegate(fd);

}

FileView::~FileView()
{

    delete file_model;
    delete contextMenu;
    delete fd;

}

void FileView::onCustomContextMenu(const QPoint &point)
{
    QModelIndex index = indexAt(point);
    if (index.isValid()) {
        contextMenu->exec(viewport()->mapToGlobal(point));
    }
}

void FileView::invertSelection()
{
     QModelIndex topLeft;
     QModelIndex bottomRight;

     topLeft = file_model->index(0, 0, QModelIndex());
     bottomRight = file_model->index(file_model->rowCount()-1,
                                     file_model->columnCount()-1, QModelIndex());

    QItemSelection selection(topLeft,bottomRight);
    selectionModel()->select(selection, QItemSelectionModel::Toggle);
}

void FileView::copy_path()
{
    QItemSelectionModel *select = selectionModel();
    QModelIndexList indexes = select->selectedIndexes();
    QModelIndex index;

    QClipboard *clipboard = QGuiApplication::clipboard();

    QString tags = "";
    bool multiple = false;

    foreach(index, indexes) {
        QString str = file_model->file(index)->path;
        if (multiple){
            tags += "\n" + str;
        } else {
            tags = str;

            multiple = true;
        }
    }

    clipboard->setText(tags);

}

void FileView::remove_file()
{
    QItemSelectionModel *select = selectionModel();
    QModelIndexList indexes = select->selectedIndexes();
    QModelIndex index;

    PyrosTC *ptc = PyrosTC::get();
    QVector<PyrosFile*> files;
    QVector<QByteArray> hashes;

    foreach(index,indexes){
        PyrosFile *file = file_model->file(index);
        if (file != NULL){
            files.append(Pyros_Duplicate_File(file));
            hashes.append(file->hash);
        }
    }

    hide_file();

    if (files.length() >= 1){
        ptc->delete_file(files);
        emit files_removed(hashes);
    }
}

void FileView::hide_files_by_hash(QVector<QByteArray> hashes){
    int old_row_count = file_model->rowCount();
    QVector<FileModel::file_item> files = file_model->files();
    QItemSelectionModel *select = selectionModel();
    QModelIndexList indexes = select->selectedIndexes();

    select->clear();

    for (int i  = files.length()-1;i >= 0;i--) {
        PyrosFile *file = files.at(i).pFile;
        if (file == nullptr)
            continue;

        for (int j  = hashes.length()-1;j >= 0;j--) {
            if (!hashes.at(j).compare(file->hash)){
                file_model->remove_file(file_model->numToIndex(i));
                hashes.removeAt(j);

                for (int k  = indexes.length()-1;k >= 0;k--) {
                    int cur_index_num = file_model->indexToNum(indexes[k]);
                    if (cur_index_num == i)
                        indexes.removeAt(k);
                    else if (cur_index_num > i)
                        indexes.replace(k,file_model->numToIndex(cur_index_num-1));
                }

                break;
            }
        }
    }

    foreach(QModelIndex index,indexes)
        select->select(index,QItemSelectionModel::Select);


    emit new_files(file_model->files());
    file_model->remove_excess_rows(old_row_count);
}

void FileView::hide_file()
{
    QItemSelectionModel *select = selectionModel();
    QModelIndexList indexes = select->selectedIndexes();
    QModelIndex index;
    int old_row_count = file_model->rowCount();

    std::sort(indexes.begin(), indexes.end(), std::less<QModelIndex>());

    for (int i = indexes.length()-1; i >= 0;i--){
        index = indexes.at(i);
        file_model->remove_file(index);
    }
    select->clear();
    emit new_files(file_model->files());
    file_model->remove_excess_rows(old_row_count);

}

void FileView::refresh(){
    PyrosTC *ptc = PyrosTC::get();

    PyrosTC::search_cb cb = [&](QVector<PyrosFile*> files){
        clear();
        file_model->setFilesFromVector(files);
        emit new_files(file_model->files());
    };

    ptc->search(this,m_tags, cb);
}

void FileView::search(QVector<QByteArray> tags)
{
    PyrosTC *ptc = PyrosTC::get();

    clear();
    scrollToTop();

    m_tags += tags;

    PyrosTC::search_cb cb = [&](QVector<PyrosFile*> files)
    {
        file_model->setFilesFromVector(files);
        emit new_files(file_model->files());
    };

    ptc->search(this,m_tags, cb);
}

void FileView::set_files_from_vector(QVector<PyrosFile*> &files)
{
    clear();

    file_model->setFilesFromVector(files);
    emit new_files(file_model->files());
}

void FileView::clear_tags()
{
    file_model->clear();
    m_tags.clear();
}

void FileView::clear()
{
    selectionModel()->clear();
    file_model->clear();
}

void FileView::add_tag(QVector<QByteArray> tags)
{
    QItemSelectionModel *select = selectionModel();
    QModelIndexList indexes = select->selectedIndexes();
    QModelIndex index;

    PyrosFile *file;
    QVector<QByteArray> hashes;
    PyrosTC *ptc = PyrosTC::get();


    foreach(index, indexes) {
        file = file_model->file(index);
        if (file != nullptr)
            hashes.push_back(file->hash);
    }

    ptc->add_tags(hashes,tags);
}

void FileView::remove_tag(QVector<QByteArray> tags)
{
    QItemSelectionModel *select = selectionModel();
    QModelIndexList indexes = select->selectedIndexes();
    PyrosTC *ptc = PyrosTC::get();

    QVector<QByteArray> files;

    foreach(QModelIndex index, indexes) {
        PyrosFile *file = file_model->file(index);
        if (file != nullptr)
            files.push_back(file->hash);

    }
    ptc->remove_tags(files,tags);
}

void FileView::remove_tag_from_search(QVector<QByteArray> tags)
{
    foreach(QByteArray tag,tags){
        for(int i = 0; i < m_tags.length(); i++){
            if(!tag.compare(m_tags[i]))
                m_tags.remove(i);
        }
    }

}

PyrosFile *FileView::file(const QModelIndex &index)
{
    return file_model->file(index);
}

QVector<PyrosFile*> FileView::files()
{
    QVector<PyrosFile*> return_files;
    QVector<FileModel::file_item> files = file_model->files();

    foreach(FileModel::file_item item,files)
        return_files.append(item.pFile);

    return return_files;
}

void FileView::get_visible()
{
    QRect rec = viewport()->rect();
    QModelIndex topLeft = indexAt(rec.topLeft());

    file_model->load_thumbnails(topLeft,height()/256);
}

void FileView::regenerate_thumbnail()
{
    QModelIndexList indexes = selectionModel()->selectedIndexes();

    foreach(QModelIndex index,indexes){
        PyrosFile *pFile = file(index);
        if (pFile != nullptr)
            FileModel::delete_thumbnail(file_model->file(index)->path);
    }
    file_model->load_thumbnails(indexes);
}


void FileView::resizeEvent(QResizeEvent *event){
    int columns = event->size().width()/256;
    int old_columns = file_model->columnCount();


    QRect rec = viewport()->rect();
    QModelIndex center = indexAt(rec.center());
    int center_file_index = file_model->indexToNum(center);
    QRect file_rec = visualRect(center);
    int excess = (file_rec.y());


    QItemSelectionModel *select = selectionModel();
    QModelIndexList indexes = select->selectedIndexes();
    QVector<int> file_indexes;

    foreach(QModelIndex index,indexes)
        file_indexes.append(file_model->indexToNum(index));


    file_model->setColumnCount(columns);
    for (int i = 0; i < columns; ++i)
        setColumnWidth(i,event->size().width()%256 /columns+256);


    QTableView::resizeEvent(event);
    if (old_columns < columns)
        launch_timer();

    if (old_columns != columns){
        //updates scroll bar
        QScrollBar *vscroll = verticalScrollBar();
        if (vscroll->value() > 0){
            QModelIndex new_pos = file_model->numToIndex(center_file_index);
            if (center.row() != new_pos.row())
                vscroll->setValue(256*(new_pos.row())-excess);
        }

        //updates file selection
        select->clear();
        foreach(int index,file_indexes){
            select->select(file_model->numToIndex(index),QItemSelectionModel::Select);
        }
    }
}

void FileView::launch_timer(){
    if(thumbtimer.isActive()){
        thumbtimer.stop();
        thumbtimer.start(150);
    } else {
        thumbtimer.start(150);
    }
}

