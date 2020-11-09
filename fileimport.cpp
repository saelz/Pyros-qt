#include "fileimport.h"
#include "searchtab.h"
#include "ui_fileimport.h"
#include "pyrosqt.h"
#include "pyrosdb.h"



#include <QDebug>
#include <QFileDialog>
#include <QStandardItemModel>

FileImport::FileImport(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FileImport)
{
    ui->setupUi(this);

    QStandardItemModel *model = new QStandardItemModel(0,1);
    ui->selected_files->setModel(model);

    connect(ui->select_button,   &QPushButton::released,this,&FileImport::add_files);
    connect(ui->add_files_button,&QPushButton::released,this,&FileImport::import_files);

    connect(ui->lineEdit, &TagLineEdit::tag_entered,this,&FileImport::add_tags);
    connect(ui->lineEdit, &TagLineEdit::tag_entered,ui->import_tags,&TagView::add_tags);

    connect(ui->import_tags, &TagView::removeTag,this, &FileImport::remove_tags);
}

FileImport::~FileImport()
{
    delete ui;
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

void FileImport::add_files()
{
    QStringList files = QFileDialog::getOpenFileNames(this,"Select Files","/home/");

    if (files.count() > 0){
        QAbstractItemModel *model = ui->selected_files->model();
        int i = 0;
        model->insertRows(0,files.count(),QModelIndex());

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

        qDebug() << file << "\n";
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

