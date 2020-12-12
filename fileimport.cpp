#include "fileimport.h"
#include "searchtab.h"
#include "ui_fileimport.h"
#include "pyrosdb.h"

#include <QDebug>
#include <QFileDialog>
#include <QMenu>
#include <QStandardItemModel>
#include <QDropEvent>

QString FileImport::starting_dir = QDir::home().path();

FileImport::FileImport(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FileImport)
{
    ui->setupUi(this);

    QStandardItemModel *model = new QStandardItemModel(0,1);
    ui->selected_files->setModel(model);

    filecontextMenu = new QMenu(this);
    ui->selected_files->setContextMenuPolicy(Qt::CustomContextMenu);
    filecontextMenu->addAction("Remove File",this,&FileImport::remove_selected_files);

    connect(ui->selected_files, &QListView::customContextMenuRequested, this, &FileImport::create_file_context_menu);

    connect(ui->select_button,   &QPushButton::released,this,&FileImport::add_files);
    connect(ui->add_files_button,&QPushButton::released,this,&FileImport::import_files);

    connect(ui->lineEdit, &TagLineEdit::tag_entered,this,&FileImport::add_tags);
    connect(ui->lineEdit, &TagLineEdit::tag_entered,ui->import_tags,&TagView::add_tags);

    connect(ui->import_tags, &TagView::removeTag,this, &FileImport::remove_tags);
    connect(ui->import_tags, &TagView::new_search_with_selected_tags,this, &FileImport::new_search_with_tags);

    setAcceptDrops(true);
}

FileImport::~FileImport()
{
    delete filecontextMenu;
    delete ui;
}

void FileImport::create_file_context_menu(const QPoint &point)
{
    QModelIndex index = ui->selected_files->indexAt(point);
    if (index.isValid()) {
        filecontextMenu->exec(ui->selected_files->viewport()->mapToGlobal(point));
    }
}


void
FileImport::remove_selected_files(){
    QItemSelectionModel *select = ui->selected_files->selectionModel();
    QModelIndexList indexes = select->selectedIndexes();
    QAbstractItemModel *model = ui->selected_files->model();

    std::sort(indexes.begin(), indexes.end(), std::less<QModelIndex>());

    for (int i = indexes.length()-1; i >= 0;i--){
        QModelIndex index = indexes.at(i);
        model->removeRow(index.row());
    }


}

void FileImport::add_tags(QVector<QByteArray> tags){
    m_import_tags += tags;
}

void FileImport::remove_tags(QVector<QByteArray> tags){
    foreach(QByteArray tag,tags){
        for(int i = 0; i < m_import_tags.length(); i++){
            if(!tag.compare(m_import_tags[i])){
                m_import_tags.remove(i);
            }
        }
    }
}

void FileImport::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls())
        event->acceptProposedAction();
}

void FileImport::dropEvent(QDropEvent* event)
{
    const QMimeData* mimeData = event->mimeData();

    if (mimeData->hasUrls()){
        foreach(QUrl url,mimeData->urls()){
            QAbstractItemModel *model = ui->selected_files->model();

            if (url.isLocalFile()){
                model->insertRows(0,1,QModelIndex());
                model->setData(model->index(0,0),url.path());
            }
        }
        event->acceptProposedAction();
    }
}

void FileImport::add_files()
{
    QStringList files = QFileDialog::getOpenFileNames(this,"Select Files",starting_dir);

    if (files.count() > 0){
        QAbstractItemModel *model = ui->selected_files->model();
        int i = 0;
        model->insertRows(0,files.count(),QModelIndex());
        QString dir = files.first();
        dir.truncate(dir.lastIndexOf('/'));
        starting_dir = dir;

        foreach (QString file ,files){
            const QModelIndex index = model->index(i,0);
            model->setData(index,file);
            i++;
        }
        ui->add_files_button->setEnabled(true);
    }
}

void FileImport::import_files()
{
    QAbstractItemModel *model = ui->selected_files->model();
    int rowCount = model->rowCount();
    QVector<QByteArray> files;
    PyrosTC *ptc = PyrosTC::get();

    ui->add_files_button->setEnabled(false);
    ui->import_progress->setMaximum(rowCount);

    for (int i = 0; i < rowCount; i++) {
        const QModelIndex index = model->index(i,0);
        const QByteArray  file = model->data(index).toByteArray();

        files.push_back(file);
    }

    PyrosTC::search_cb cb = [&](QVector<PyrosFile*> files){
        emit new_search(files);
        delete this;
    };

    PyrosTC::import_progress_cb progress_cb = [&](int i){
        ui->import_progress->setValue(i);
    };

    ptc->import(this,files,cb,progress_cb,
                ui->use_tag_files->isChecked(),m_import_tags);

}

