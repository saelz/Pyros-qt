#include "fileview.h"
#include "pyrosqt.h"
#include "pyrosdb.h"

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

#include <iostream>


class FileDelegate : public QStyledItemDelegate {
  public:
    FileModel *model;
    FileDelegate(FileModel *model,QObject *parent )
        : QStyledItemDelegate(parent),model(model) {}

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
                const QModelIndex &index) const {
      QStyledItemDelegate::paint(painter, option, index);
      //QColor k = model->data(index,Qt::ForegroundRole).toUInt();
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
      //painter->drawRect(option.rect);
      QRect rect = option.rect;
      rect.setTop(rect.top()+1);
      rect.setLeft(rect.left()+1);
      rect.setWidth(rect.width()-1);
      rect.setHeight(rect.height()-1);
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

    contextMenu->addAction("Delete File",   this,&FileView::remove_file);
    contextMenu->addAction("Hide File",     this,&FileView::hide_file);
    contextMenu->addAction("Copy file path",this,&FileView::copy_path);

    connect(this, &FileView::customContextMenuRequested, this, &FileView::onCustomContextMenu);
    connect(this,&FileView::new_files,this, &FileView::get_visible);
    connect(verticalScrollBar(), &QScrollBar::valueChanged, this, &FileView::launch_timer);
    connect(&thumbtimer, &QTimer::timeout, this, &FileView::get_visible);

    fd = new FileDelegate(file_model,this);
    setItemDelegate(fd);

}

FileView::~FileView()
{

    //for( char *t : m_tags) delete[] t;
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

    foreach(index,indexes){
        PyrosFile *file = file_model->file(index);
        if (file != NULL)
            files.append(Pyros_Duplicate_File(file));
    }

    if (files.length() >= 1)
        ptc->delete_file(files);
    hide_file();
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
        file_model->clear();
        file_model->setFilesFromVector(files);
        emit new_files(file_model->files());
    };

    ptc->search(this,m_tags, cb);

    //emit new_files(file_model->files());

}

void FileView::search(QVector<QByteArray> tags)
{
    PyrosTC *ptc = PyrosTC::get();

    file_model->clear();
    scrollToTop();

    m_tags += tags;

    PyrosTC::search_cb cb = [&](QVector<PyrosFile*> files){
        file_model->setFilesFromVector(files);
        emit new_files(file_model->files());
    };

    ptc->search(this,m_tags, cb);
}

void FileView::set_files_from_vector(QVector<PyrosFile*> &files){
    file_model->clear();

    file_model->setFilesFromVector(files);
    emit new_files(file_model->files());
}

void FileView::clear_tags(){
    file_model->clear();
    m_tags.clear();
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
            if(!tag.compare(m_tags[i])){
                //delete[] m_tags[i];
                m_tags.remove(i);
            }
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


void FileView::resizeEvent(QResizeEvent *event){
    int columns = event->size().width()/256;
    int old_columns = file_model->columnCount();


    file_model->setColumnCount(columns);
    for (int i = 0; i < columns; ++i) {
        setColumnWidth(i,event->size().width()%256 /columns+256);
    }


    QTableView::resizeEvent(event);
    if (old_columns < columns)
        launch_timer();

}

void FileView::launch_timer(){
    if(thumbtimer.isActive()){
        thumbtimer.stop();
        thumbtimer.start(150);
    } else {
        thumbtimer.start(150);
    }
}

