#include "fileviewer.h"
#include "pyrosqt.h"
#include "pyrosdb.h"
#include "MediaViewer/mediaviewer.h"

#include "ui_fileviewer.h"

#include "tagtreemodel.h"
#include "configtab.h"

#include <QComboBox>
#include <QResizeEvent>
#include <QSettings>
#include <QScrollBar>

using ct = configtab;

FileViewer::FileViewer(QVector<PyrosFile*> files,int inital_pos,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FileViewer),
    position(inital_pos)
{

    ui->setupUi(this);

    foreach(PyrosFile *pFile,files){
        m_files.push_back(Pyros_Duplicate_File(pFile));
    }



    QAction *next_bind = ct::create_binding(ct::KEY_NEXT_FILE,"Next file",this);
    QAction *prev_bind = ct::create_binding(ct::KEY_PREV_FILE,"Previous file",this);
    QAction *insert_bind = ct::create_binding(ct::KEY_FOCUS_TAG_BAR,"Insert",this);
    QAction *delete_bind = ct::create_binding(ct::KEY_DELETE_FILE,"Delete",this);
    QAction *zoom_in_bind = ct::create_binding(ct::KEY_ZOOM_IN,"Zoom in",this);
    QAction *zoom_out_bind = ct::create_binding(ct::KEY_ZOOM_OUT,"Zoom out",this);
    QAction *next_page_bind = ct::create_binding(ct::KEY_NEXT_PAGE,"Next page",this);
    QAction *prev_page_bind = ct::create_binding(ct::KEY_PREV_PAGE,"Previous page",this);
    QAction *focus_file_viewer = ct::create_binding(ct::KEY_FOCUS_FILE_VIEWER,"Focus file viewer",this);

    connect(next_bind,   &QAction::triggered,this, &FileViewer::next_file);
    connect(prev_bind,   &QAction::triggered,this, &FileViewer::prev_file);
    connect(insert_bind, &QAction::triggered,this, &FileViewer::select_tag_bar);
    connect(delete_bind, &QAction::triggered,this, &FileViewer::delete_file);
    connect(zoom_in_bind, &QAction::triggered,ui->mediaviewer, &MediaViewer::zoom_in);
    connect(zoom_out_bind, &QAction::triggered,ui->mediaviewer, &MediaViewer::zoom_out);
    connect(next_page_bind, &QAction::triggered,ui->mediaviewer, &MediaViewer::next_page);
    connect(prev_page_bind, &QAction::triggered,ui->mediaviewer, &MediaViewer::prev_page);
    connect(focus_file_viewer, &QAction::triggered,ui->mediaviewer, &MediaViewer::set_focus);


    connect(ui->fit_combo_box, &QComboBox::currentTextChanged,this, &FileViewer::update_fit);

    connect(ui->next_button,   &QPushButton::released,this, &FileViewer::next_file);
    connect(ui->prev_button,   &QPushButton::released,this, &FileViewer::prev_file);
    connect(ui->delete_button, &QPushButton::released,this, &FileViewer::delete_file);

    connect(ui->zoom_in_button,  &QPushButton::released,ui->mediaviewer, &MediaViewer::zoom_in);
    connect(ui->zoom_out_button, &QPushButton::released,ui->mediaviewer, &MediaViewer::zoom_out);
    connect(ui->tag_bar, &TagLineEdit::tag_entered,ui->file_tags, &TagView::add_tags);
    connect(ui->tag_bar, &TagLineEdit::tag_entered,this, &FileViewer::add_tag);

    connect(ui->file_tags, &TagView::removeTag, this,&FileViewer::remove_tag);
    connect(ui->file_tags, &TagView::new_search_with_selected_tags,this, &FileViewer::new_search_with_selected_tags);

    connect(ui->cbz_prev_page, &QPushButton::released,ui->mediaviewer, &MediaViewer::prev_page);
    connect(ui->cbz_next_page, &QPushButton::released,ui->mediaviewer, &MediaViewer::next_page);

    connect(ui->mediaviewer,&MediaViewer::info_updated,ui->file_info,&QLabel::setText);

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
        ui->file_count_label->setText("0");
        return;
    }

    QString filecount = QString::number(position+1)+"/"+QString::number(m_files.count());
    ui->file_count_label->setText(filecount);

    ui->mediaviewer->set_file(m_pFile);

    ui->cbz_buttons->setVisible(ui->mediaviewer->is_multipaged());
    ui->image_buttons->setVisible(ui->mediaviewer->is_resizable());

    ui->file_tags->setTagsFromFile(m_pFile);
}

void FileViewer::update_fit(const QString &text)
{
    if (text == "Fit Both")
        ui->mediaviewer->set_scale(MediaViewer::SCALE_TYPE::BOTH);
    else if (text == "Fit Width")
        ui->mediaviewer->set_scale(MediaViewer::SCALE_TYPE::WIDTH);
    else if (text == "Fit Height")
        ui->mediaviewer->set_scale(MediaViewer::SCALE_TYPE::HEIGHT);
    else if (text == "Original")
        ui->mediaviewer->set_scale(MediaViewer::SCALE_TYPE::ORIGINAL);
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
                if (position > i)
                    position--;
                if (position < 0)
                    position = 0;
                hashes.removeAt(j);
            }
        }

    }

    set_file();
}
