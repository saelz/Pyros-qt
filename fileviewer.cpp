#include "fileviewer.h"
#include "pyrosqt.h"
#include "pyrosdb.h"

#include "ui_fileviewer.h"

#include "tagtreemodel.h"

#include <QImageReader>
#include <QMovie>
#include <QBitmap>
#include <QComboBox>
#include <QResizeEvent>

#include <iostream>


FileViewer::FileViewer(QVector<PyrosFile*> files,int inital_pos,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FileViewer),
    position(inital_pos)
{

    ui->setupUi(this);

    foreach(PyrosFile *pFile,files){
        m_files.push_back(Pyros_Duplicate_File(pFile));
    }

    set_file();


    QAction *next_bind;
    QAction *prev_bind;
    QAction *insert_bind;
    QAction *delete_bind;
    QAction *zoom_in_bind;
    QAction *zoom_out_bind;

    next_bind = new QAction("Next File",this);
    prev_bind = new QAction("Prev File",this);
    insert_bind = new QAction("Insert",this);
    delete_bind = new QAction("delete",this);
    zoom_in_bind = new QAction("zoom in",this);
    zoom_out_bind = new QAction("zoom out",this);

    next_bind->setShortcut(QKeySequence("CTRL+n"));
    prev_bind->setShortcut(QKeySequence("CTRL+p"));
    insert_bind->setShortcut(QKeySequence("i"));
    delete_bind->setShortcut(QKeySequence("CTRL+del"));
    next_bind->setShortcut(QKeySequence("CTRL+n"));
    prev_bind->setShortcut(QKeySequence("CTRL+p"));
    zoom_in_bind->setShortcut(QKeySequence("CTRL++"));
    zoom_out_bind->setShortcut(QKeySequence("CTRL+-"));

    connect(next_bind,   &QAction::triggered,this, &FileViewer::next_file);
    connect(prev_bind,   &QAction::triggered,this, &FileViewer::prev_file);
    connect(insert_bind, &QAction::triggered,this, &FileViewer::select_tag_bar);
    connect(delete_bind, &QAction::triggered,this, &FileViewer::delete_file);
    connect(zoom_in_bind, &QAction::triggered,this, &FileViewer::zoom_in);
    connect(zoom_out_bind, &QAction::triggered,this, &FileViewer::zoom_out);

    //connect insert_bind
    addAction(next_bind);
    addAction(prev_bind);
    addAction(insert_bind);
    addAction(delete_bind);
    addAction(zoom_in_bind);
    addAction(zoom_out_bind);

    connect(ui->fit_combo_box, &QComboBox::currentTextChanged,this, &FileViewer::update_fit);

    connect(ui->next_button,   &QPushButton::released,this, &FileViewer::next_file);
    connect(ui->prev_button,   &QPushButton::released,this, &FileViewer::prev_file);
    connect(ui->delete_button, &QPushButton::released,this, &FileViewer::delete_file);

    connect(ui->zoom_in_button,  &QPushButton::released,this, &FileViewer::zoom_in);
    connect(ui->zoom_out_button, &QPushButton::released,this, &FileViewer::zoom_out);

    connect(ui->tag_bar, &TagLineEdit::tag_entered,ui->file_tags, &TagView::add_tags);
    connect(ui->tag_bar, &TagLineEdit::tag_entered,this, &FileViewer::add_tag);

    connect(ui->file_tags, &TagView::removeTag, this,&FileViewer::remove_tag);

    ui->scrollArea->installEventFilter(this);
}

FileViewer::~FileViewer()
{
    foreach(PyrosFile *pFile,m_files) Pyros_Close_File(pFile);
    ui->mpv_player->stop();
    delete ui;
}

void FileViewer::select_tag_bar()
{
    ui->tag_bar->setFocus(Qt::OtherFocusReason);
}

void FileViewer::set_file()
{
    m_pFile = m_files.at(position);
    ui->mpv_player->stop();
    zoom_level = 1;

    if (m_pFile == nullptr)
        return;

    QString filecount = QString::number(position+1)+"/"+QString::number(m_files.count());
    ui->file_count_label->setText(filecount);

    if (!strcmp(m_pFile->mime,"image/gif") ||
        !strncmp(m_pFile->mime,"audio/",6) ||
        !strncmp(m_pFile->mime,"video/",6)){
        viewer_type = MPV;

        ui->mpv_player->set_file(m_pFile->path);
        ui->stackedWidget->setCurrentIndex(1);
    } else if (!strncmp(m_pFile->mime,"image/",6)){
        viewer_type = IMAGE;
        m_img = QPixmap(m_pFile->path);

        ui->img_label->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
        ui->stackedWidget->setCurrentIndex(0);
        set_scale();
    } else if (!strcmp(m_pFile->mime,"text/plain")){
        viewer_type = TEXT;
        ui->stackedWidget->setCurrentIndex(0);

        QFont f("Arial", font_size);
        m_img = QPixmap();

        ui->img_label->setFont(f);
        ui->img_label->setPixmap(m_img);
        ui->img_label->setWordWrap(true);
        ui->img_label->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);

        QFile file(m_pFile->path);
        QString line;
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)){
            QTextStream stream(&file);
            while (!stream.atEnd()){

                line.append(stream.readLine()+"\n");
            }
            ui->img_label->setText(line);
        }
        file.close();
    } else {
        viewer_type = UNSUPPORTED;
        ui->stackedWidget->setCurrentIndex(0);

        QFont f("Arial", font_size);
        m_img = QPixmap();

        ui->img_label->setFont(f);
        ui->img_label->setPixmap(m_img);
        ui->img_label->setWordWrap(true);
        ui->img_label->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
        ui->img_label->setText("Unsupported filetype");

    }

    ui->file_tags->setTagsFromFile(m_pFile);


}

void FileViewer::update_fit(const QString &text){
    zoom_level = 1;
    if (text == "Fit Both")
        scale_type = SCALE_TYPE::BOTH;
    else if (text == "Fit Width")
        scale_type = SCALE_TYPE::WIDTH;
    else if (text == "Fit Height")
        scale_type = SCALE_TYPE::HEIGHT;
    else if (text == "Original")
        scale_type = SCALE_TYPE::ORIGINAL;

    set_scale();
}

void FileViewer::zoom_in(){
    if (viewer_type == IMAGE){
        if ((zoom_level += zoom_increment) >= 3)
            zoom_level = 3;
        set_scale();
    } else if (viewer_type == TEXT){
        font_size++;
        QFont f("Arial", font_size);
        ui->img_label->setFont(f);
    }
}

void FileViewer::zoom_out(){
    if (viewer_type == IMAGE){
        if ((zoom_level -= zoom_increment) <= 0)
            zoom_level = zoom_increment;
        set_scale();
    } else if (viewer_type == TEXT){
        if ((font_size--) == 0)
            font_size = 1;
        QFont f("Arial", font_size);
        ui->img_label->setFont(f);
    }
}

void FileViewer::set_scale()
{
    if (viewer_type != IMAGE)
        return;
    QPixmap scaledimg;

    switch (scale_type){
    case SCALE_TYPE::HEIGHT:
        scaledimg = m_img.scaled(m_img.width()*zoom_level,ui->scrollArea->height()*zoom_level,
                     Qt::KeepAspectRatio, Qt::SmoothTransformation);
        break;
    case SCALE_TYPE::WIDTH:
        scaledimg = m_img.scaled(ui->scrollArea->width()*zoom_level,m_img.height()*zoom_level,
                     Qt::KeepAspectRatio, Qt::SmoothTransformation);
        break;
    case SCALE_TYPE::BOTH:
        scaledimg = m_img.scaled(ui->scrollArea->width()*zoom_level,ui->scrollArea->height()*zoom_level,
                     Qt::KeepAspectRatio, Qt::SmoothTransformation);

        break;
    case SCALE_TYPE::ORIGINAL:
        scaledimg = m_img.scaled(m_img.width()*zoom_level,m_img.height()*zoom_level,
                     Qt::KeepAspectRatio, Qt::SmoothTransformation);
        break;
    }
    ui->img_label->setPixmap(scaledimg);
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

    ptc->delete_file(pFile);
    m_files.removeAt(position);
    if (position >= m_files.size())
        position--;
    if (position < 0)
        position = 0;

    set_file();

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


bool FileViewer::eventFilter(QObject *obj, QEvent *event)
{
    bool result = QWidget::eventFilter(obj, event);
    if (viewer_type == IMAGE &&
            event->type() == QEvent::Resize &&
            obj == ui->scrollArea) {
        if (scale_type != SCALE_TYPE::ORIGINAL ||
                zoom_level != 1){
            set_scale();
        }
    }

    return result;
}
