#include "fileviewer.h"
#include "pyrosqt.h"
#include "pyrosdb.h"

#include "MediaViewer/Overlay/overlay.h"
#include "MediaViewer/Overlay/overlay_button.h"
#include "MediaViewer/mediaviewer.h"

#include "ui_fileviewer.h"

#include "tagtreemodel.h"
#include "configtab.h"

#include <QComboBox>
#include <QResizeEvent>
#include <QSettings>
#include <QScrollBar>

using ct = configtab;

FileViewer::FileViewer(QVector<PyrosFile*> files,int inital_pos,QTabWidget *parent) :
    Tab(parent),
    ui(new Ui::FileViewer)
{

    ui->setupUi(this);
    set_title("Viewer");

    QVector<PyrosFile *> new_files;
    foreach(PyrosFile *pFile,files)
        new_files.push_back(Pyros_Duplicate_File(pFile));

    QAction *insert_bind = ct::create_binding(ct::KEY_FOCUS_TAG_BAR,"Insert",this);

    connect(insert_bind, &QAction::triggered,this, &FileViewer::select_tag_bar);

    connect(ui->tag_bar, &TagLineEdit::tag_entered,ui->file_tags, &TagView::add_tags);
    connect(ui->tag_bar, &TagLineEdit::tag_entered,this, &FileViewer::add_tag);

    connect(ui->file_tags, &TagView::removeTag, this,&FileViewer::remove_tag);
    connect(ui->file_tags, &TagView::new_search_with_selected_tags,this, &FileViewer::new_search_with_selected_tags);

    connect(this,&FileViewer::hide_files,ui->mediaviewer,&MediaViewer::hide_files);
    connect(ui->mediaviewer,&MediaViewer::file_deleted,this,&FileViewer::file_deleted);
    connect(ui->mediaviewer,&MediaViewer::file_changed,this,&FileViewer::set_file);

    ui->mediaviewer->bind_keys(this,true);
    ui->mediaviewer->set_files(new_files,inital_pos);
}


FileViewer::~FileViewer()
{
    delete ui;
}

void FileViewer::select_tag_bar()
{
    ui->tag_bar->setFocus(Qt::OtherFocusReason);
}


void FileViewer::set_file(PyrosFile *file)
{
    ui->file_tags->clear();
    m_pFile = file;
    ui->file_tags->setTagsFromFile(m_pFile);
}


void FileViewer::add_tag(QVector<QByteArray> tags)
{
    if (m_pFile == nullptr)
        return;

    PyrosTC *ptc = PyrosTC::get();
    ptc->add_tags(QByteArray(m_pFile->hash),tags);
}

void FileViewer::remove_tag(QVector<QByteArray> tags)
{
    PyrosTC *ptc = PyrosTC::get();
    ptc->remove_tags(m_pFile->hash,tags);
}
