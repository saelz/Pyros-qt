#include "fileviewer.h"
#include "pyrosqt.h"
#include "pyrosdb.h"

#include "MediaViewer/Overlay/overlay.h"
#include "MediaViewer/Overlay/overlay_text.h"
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
    ui(new Ui::FileViewer),
    position(inital_pos)
{

    ui->setupUi(this);
    set_title("Viewer");

    foreach(PyrosFile *pFile,files){
        m_files.push_back(Pyros_Duplicate_File(pFile));
    }

    Overlay_Text *overlay_file_count = new Overlay_Text("File count",ui->mediaviewer->overlay);
    Overlay_Button *overlay_delete_button = new Overlay_Button(":/data/icons/trash.png",nullptr,"Delete file",ui->mediaviewer->overlay);

    Overlay_Button *overlay_next_button = new Overlay_Button(":/data/icons/right_arrow.png",nullptr,"Next file",ui->mediaviewer->overlay);
    Overlay_Button *overlay_prev_button = new Overlay_Button(":/data/icons/left_arrow.png",nullptr,"Prev file",ui->mediaviewer->overlay);

    ui->mediaviewer->overlay->main_bar.widgets.prepend(overlay_next_button);
    ui->mediaviewer->overlay->main_bar.widgets.prepend(overlay_prev_button);

    ui->mediaviewer->overlay->main_bar.widgets.append(overlay_file_count);
    ui->mediaviewer->overlay->main_bar.widgets.append(overlay_delete_button);

    connect(overlay_delete_button,&Overlay_Button::clicked,this,&FileViewer::delete_file);

    connect(overlay_next_button,&Overlay_Button::clicked,this,&FileViewer::next_file);
    connect(overlay_prev_button,&Overlay_Button::clicked,this,&FileViewer::prev_file);

    connect(this,&FileViewer::update_file_count,overlay_file_count,&Overlay_Text::set_text);

    QAction *next_bind = ct::create_binding(ct::KEY_NEXT_FILE,"Next file",this);
    QAction *prev_bind = ct::create_binding(ct::KEY_PREV_FILE,"Previous file",this);
    QAction *insert_bind = ct::create_binding(ct::KEY_FOCUS_TAG_BAR,"Insert",this);
    QAction *delete_bind = ct::create_binding(ct::KEY_DELETE_FILE,"Delete",this);

    connect(next_bind,   &QAction::triggered,this, &FileViewer::next_file);
    connect(prev_bind,   &QAction::triggered,this, &FileViewer::prev_file);
    connect(insert_bind, &QAction::triggered,this, &FileViewer::select_tag_bar);
    connect(delete_bind, &QAction::triggered,this, &FileViewer::delete_file);

    connect(ui->tag_bar, &TagLineEdit::tag_entered,ui->file_tags, &TagView::add_tags);
    connect(ui->tag_bar, &TagLineEdit::tag_entered,this, &FileViewer::add_tag);

    connect(ui->file_tags, &TagView::removeTag, this,&FileViewer::remove_tag);
    connect(ui->file_tags, &TagView::new_search_with_selected_tags,this, &FileViewer::new_search_with_selected_tags);

    ui->mediaviewer->bind_keys(this);

    set_file();
}


FileViewer::~FileViewer()
{
    foreach(PyrosFile *pFile,m_files) Pyros_Close_File(pFile);
    delete ui;
}

void FileViewer::select_tag_bar()
{
    ui->tag_bar->setFocus(Qt::OtherFocusReason);
}


void FileViewer::set_file()
{
    m_pFile = m_files.at(position);

    ui->file_tags->clear();


    if (m_pFile == nullptr || m_files.count() <= 0){
        ui->mediaviewer->set_file(nullptr);
        emit update_file_count("0");
        return;
    }

    emit update_file_count(QString::number(position+1)+"/"+QString::number(m_files.count()));

    ui->mediaviewer->set_file(m_pFile);

    ui->file_tags->setTagsFromFile(m_pFile);
}

void FileViewer::next_file()
{
    if (position+1 < m_files.size()){
        position++;
        set_file();
    }
}

void FileViewer::prev_file()
{
    if (position-1 >= 0){
        position--;
        set_file();
    }
}

void FileViewer::delete_file()
{
    PyrosTC *ptc = PyrosTC::get();
    PyrosFile *pFile = m_files.at(position);

    if (pFile == nullptr || m_files.size() == 0)
        return;

    QVector<QByteArray> file_hashes;
    file_hashes.append(pFile->hash);


    ptc->delete_file(pFile);
    m_files.removeAt(position);
    if (position >= m_files.size())
        position--;
    if (position < 0)
        position = 0;

    set_file();
    emit file_deleted(file_hashes);

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

void FileViewer::hide_files(QVector<QByteArray> hashes){
    for (int i  = m_files.length()-1;i >= 0;i--) {
        PyrosFile *file = m_files.at(i);
        if (file == nullptr)
            continue;

        for (int j  = hashes.length()-1;j >= 0;j--) {
            if (!hashes.at(j).compare(file->hash)){
                m_files.removeAt(i);
                if (position >= i)
                    position--;
                if (position < 0)
                    position = 0;
                hashes.removeAt(j);
            }
        }

    }

    set_file();
}
